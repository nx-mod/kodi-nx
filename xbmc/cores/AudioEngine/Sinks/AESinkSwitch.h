/*
 *  Copyright (C) 2026 Team Kodi
 *  This file is part of Kodi - https://kodi.tv
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 *  See LICENSES/README.md for more information.
 */

#pragma once

#include "cores/AudioEngine/Interfaces/AESink.h"
#include "cores/AudioEngine/Utils/AEDeviceInfo.h"

#include <array>
#include <memory>

#include <switch.h>

// PCM output through audout: 48 kHz, 2 channels, signed 16-bit, a ring of 0x1000-aligned buffers.
class CAESinkSwitch : public IAESink
{
public:
  const char* GetName() override { return "switch"; }

  CAESinkSwitch() = default;
  ~CAESinkSwitch() override;

  static void Register();
  static std::unique_ptr<IAESink> Create(std::string& device, AEAudioFormat& desiredFormat);
  static void EnumerateDevicesEx(AEDeviceInfoList& list, bool force = false);

  bool Initialize(AEAudioFormat& format, std::string& device) override;
  void Deinitialize() override;

  void GetDelay(AEDelayStatus& status) override;
  double GetCacheTotal() override;
  unsigned int AddPackets(uint8_t** data, unsigned int frames, unsigned int offset) override;
  void Drain() override;

private:
  static constexpr unsigned int kBuffers = 4;
  static constexpr unsigned int kFramesPerBuffer = 1024; // 21.3 ms at 48 kHz
  static constexpr unsigned int kFrameBytes = 4; // 2 channels x s16

  bool ReclaimOne(uint64_t timeoutNs);
  void MarkReleased(AudioOutBuffer* released);
  unsigned int QueuedFrames() const;

  struct Slot
  {
    AudioOutBuffer out{};
    void* mem = nullptr;
    bool queued = false;
  };
  std::array<Slot, kBuffers> m_slots{};
  size_t m_bufferBytes = 0;
  unsigned int m_next = 0;
  bool m_open = false;
  bool m_started = false;
  uint64_t m_appendedFrames = 0; // frames handed to audout since Initialize
};
