/*
 *  Copyright (C) 2026 Team Kodi
 *  This file is part of Kodi - https://kodi.tv
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 *  See LICENSES/README.md for more information.
 */

#include "platform/posix/CPUInfoPosix.h"
#include "platform/posix/GPUInfoPosix.h"
#include "utils/GpuInfo.h"
#include "utils/MemUtils.h"
#include "utils/Temperature.h"

#include <cstdlib>
#include <memory>

#include <switch.h>

namespace
{
class CCPUInfoSwitch : public CCPUInfoPosix
{
public:
  CCPUInfoSwitch()
  {
    m_cpuModel = "ARM Cortex-A57";
    m_cpuVendor = "ARM";
    m_cpuHardware = "Nintendo Switch";
    m_cpuFeatures |= CPU_FEATURE_NEON;
  }

  bool SupportsCPUUsage() const override { return false; }
  int GetUsedPercentage() override { return 0; }
  float GetCPUFrequency() override { return 0.0f; }
  bool GetTemperature(CTemperature& temperature) override { return false; }
};

class CGPUInfoSwitch : public CGPUInfoPosix
{
public:
  bool GetGPUPlatformTemperature(CTemperature& temperature) const override { return false; }
};
} // namespace

std::shared_ptr<CCPUInfo> CCPUInfo::GetCPUInfo()
{
  return std::make_shared<CCPUInfoSwitch>();
}

std::unique_ptr<CGPUInfo> CGPUInfo::GetGPUInfo()
{
  return std::make_unique<CGPUInfoSwitch>();
}

namespace KODI
{
namespace MEMORY
{
void* AlignedMalloc(size_t s, size_t alignTo)
{
  const size_t size = (s + alignTo - 1) / alignTo * alignTo;
  return std::aligned_alloc(alignTo, size ? size : alignTo);
}

void AlignedFree(void* p)
{
  std::free(p);
}

void GetMemoryStatus(MemoryStatus* buffer)
{
  if (!buffer)
    return;

  u64 total = 0;
  u64 used = 0;
  svcGetInfo(&total, InfoType_TotalMemorySize, CUR_PROCESS_HANDLE, 0);
  svcGetInfo(&used, InfoType_UsedMemorySize, CUR_PROCESS_HANDLE, 0);
  buffer->totalPhys = total;
  buffer->availPhys = total > used ? total - used : 0;
  buffer->memoryLoad = total ? static_cast<unsigned int>(used * 100 / total) : 0;
}
} // namespace MEMORY
} // namespace KODI
