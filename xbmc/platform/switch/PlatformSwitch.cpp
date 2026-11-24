/*
 *  Copyright (C) 2026 Team Kodi
 *  This file is part of Kodi - https://kodi.tv
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 *  See LICENSES/README.md for more information.
 */

#include "PlatformSwitch.h"

#include <cstdlib>

CPlatform* CPlatform::CreateInstance()
{
  return new CPlatformSwitch();
}

bool CPlatformSwitch::InitStageOne()
{
  if (!CPlatformPosix::InitStageOne())
    return false;

  setenv("OS", "Switch", true);
  return true;
}

void CPlatformSwitch::DeinitStageOne()
{
  CPlatformPosix::DeinitStageOne();
}
