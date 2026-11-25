/*
 *  Copyright (C) 2026 Team Kodi
 *  This file is part of Kodi - https://kodi.tv
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 *  See LICENSES/README.md for more information.
 */

#include "AESinkSwitch.h"

#include "cores/AudioEngine/AESinkFactory.h"
#include "utils/log.h"

#include <algorithm>
#include <cstdlib>
#include <cstring>

namespace
{
constexpr unsigned int kRate = 48000;
constexpr size_t kAlign = 0x1000;
} // namespace

void CAESinkSwitch::Register()
{
  AE::AESinkRegEntry entry;
  entry.sinkName = "SWITCH";
  entry.createFunc = CAESinkSwitch::Create;
  entry.enumerateFunc = CAESinkSwitch::EnumerateDevicesEx;
  AE::CAESinkFactory::RegisterSink(entry);
}

std::unique_ptr<IAESink> CAESinkSwitch::Create(std::string& device, AEAudioFormat& desiredFormat)
{
  auto sink = std::make_unique<CAESinkSwitch>();
  if (sink->Initialize(desiredFormat, device))
    return sink;
  return {};
}

void CAESinkSwitch::EnumerateDevicesEx(AEDeviceInfoList& list, bool force)
{
  CAEDeviceInfo info;
  info.m_deviceName = "default";
  info.m_displayName = "Switch 2";
  info.m_displayNameExtra = "audout";
  info.m_deviceType = AE_DEVTYPE_PCM;
  info.m_channels = AE_CH_LAYOUT_2_0;
  info.m_sampleRates.push_back(kRate);
  info.m_dataFormats.push_back(AE_FMT_S16NE);
  info.m_wantsIECPassthrough = false;
  list.push_back(info);
}

CAESinkSwitch::~CAESinkSwitch()
{
  Deinitialize();
}

bool CAESinkSwitch::Initialize(AEAudioFormat& format, std::string& device)
{
  Result rc = audoutInitialize();
  if (R_FAILED(rc))
  {
    CLog::Log(LOGERROR, "CAESinkSwitch: audoutInitialize failed 0x{:x}", rc);
    return false;
  }
  rc = audoutStartAudioOut();
  if (R_FAILED(rc))
  {
    CLog::Log(LOGERROR, "CAESinkSwitch: audoutStartAudioOut failed 0x{:x}", rc);
    audoutExit();
    return false;
  }
  m_open = true;
  m_started = true;

  m_bufferBytes = (kFramesPerBuffer * kFrameBytes + kAlign - 1) & ~(kAlign - 1);
  for (Slot& slot : m_slots)
  {
    slot.mem = std::aligned_alloc(kAlign, m_bufferBytes);
    if (!slot.mem)
    {
      Deinitialize();
      return false;
    }
    std::memset(slot.mem, 0, m_bufferBytes);
    slot.out = {};
    slot.out.buffer = slot.mem;
    slot.out.buffer_size = m_bufferBytes;
    slot.queued = false;
  }
  m_next = 0;
  m_appendedFrames = 0;

  format.m_dataFormat = AE_FMT_S16NE;
  format.m_sampleRate = kRate;
  format.m_channelLayout = AE_CH_LAYOUT_2_0;
  format.m_frameSize = kFrameBytes;
  format.m_frames = kFramesPerBuffer;
  device = "default";
  return true;
}

void CAESinkSwitch::Deinitialize()
{
  if (m_started)
  {
    bool flushed = false;
    audoutFlushAudioOutBuffers(&flushed);
    audoutStopAudioOut();
    m_started = false;
  }
  if (m_open)
  {
    audoutExit();
    m_open = false;
  }
  for (Slot& slot : m_slots)
  {
    std::free(slot.mem);
    slot.mem = nullptr;
    slot.queued = false;
  }
}

void CAESinkSwitch::MarkReleased(AudioOutBuffer* released)
{
  for (Slot& slot : m_slots)
  {
    if (&slot.out == released)
    {
      slot.queued = false;
      return;
    }
  }
}

bool CAESinkSwitch::ReclaimOne(uint64_t timeoutNs)
{
  // audoutWaitPlayFinish hands back only the first released buffer; drain the rest without
  // waiting, or buffers released together are never reused and audout runs dry.
  AudioOutBuffer* released = nullptr;
  u32 count = 0;
  Result rc = audoutWaitPlayFinish(&released, &count, timeoutNs);
  if (R_FAILED(rc) || count == 0 || !released)
    return false;
  MarkReleased(released);
  for (int guard = 0; guard < static_cast<int>(kBuffers); ++guard)
  {
    AudioOutBuffer* more = nullptr;
    u32 n = 0;
    if (R_FAILED(audoutGetReleasedAudioOutBuffer(&more, &n)) || n == 0 || !more)
      break;
    MarkReleased(more);
  }
  return true;
}

unsigned int CAESinkSwitch::QueuedFrames() const
{
  unsigned int frames = 0;
  for (const Slot& slot : m_slots)
    if (slot.queued)
      frames += static_cast<unsigned int>(slot.out.data_size / kFrameBytes);
  return frames;
}

unsigned int CAESinkSwitch::AddPackets(uint8_t** data, unsigned int frames, unsigned int offset)
{
  if (!m_open)
    return 0;

  Slot& slot = m_slots[m_next];
  if (slot.queued)
  {
    // Wait at most one buffer's length for the oldest buffer to come back.
    const uint64_t oneBufferNs = 1000000000ULL * kFramesPerBuffer / kRate;
    while (slot.queued)
    {
      if (!ReclaimOne(oneBufferNs * 2))
        return 0;
    }
  }

  const unsigned int n = std::min(frames, kFramesPerBuffer);
  std::memcpy(slot.mem, data[0] + offset * kFrameBytes, n * kFrameBytes);
  slot.out.data_size = n * kFrameBytes;
  slot.out.data_offset = 0;
  if (R_FAILED(audoutAppendAudioOutBuffer(&slot.out)))
    return 0;
  slot.queued = true;
  m_appendedFrames += n;
  m_next = (m_next + 1) % kBuffers;
  return n;
}

void CAESinkSwitch::GetDelay(AEDelayStatus& status)
{
  // Collect buffers that have finished without blocking.
  while (ReclaimOne(0))
  {
  }
  // Exact delay = frames appended but not yet played (the playing buffer counts only its unplayed part).
  u64 played = 0;
  if (R_SUCCEEDED(audoutGetAudioOutPlayedSampleCount(&played)) && played <= m_appendedFrames)
    status.SetDelay(static_cast<double>(m_appendedFrames - played) / kRate);
  else
    status.SetDelay(static_cast<double>(QueuedFrames()) / kRate);
}

double CAESinkSwitch::GetCacheTotal()
{
  return static_cast<double>(kBuffers * kFramesPerBuffer) / kRate;
}

void CAESinkSwitch::Drain()
{
  while (QueuedFrames() > 0)
  {
    if (!ReclaimOne(100000000ULL))
      break;
  }
}
