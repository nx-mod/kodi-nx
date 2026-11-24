/*
 *  Copyright (C) 2026 Team Kodi
 *  This file is part of Kodi - https://kodi.tv
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 *  See LICENSES/README.md for more information.
 */

#include "application/AppEnvironment.h"
#include "application/AppParamParser.h"
#include "platform/xbmc.h"

#include <cstdlib>
#include <locale.h>
#include <sys/stat.h>
#include <unistd.h>

#include <switch.h>

namespace
{
constexpr const char* KODI_ROOT = "sdmc:/switch/kodi";
constexpr const char* KODI_HOME = "/switch/kodi";

void BringUpServices()
{
  appletSetFocusHandlingMode(AppletFocusHandlingMode_SuspendHomeSleepNotify);
  appletLockExit();
  atexit([] { appletUnlockExit(); });

  SocketInitConfig sockCfg = *socketGetDefaultInitConfig();
  sockCfg.tcp_tx_buf_size = 0x40000;
  sockCfg.tcp_rx_buf_size = 0x80000;
  sockCfg.tcp_tx_buf_max_size = 0x100000;
  sockCfg.tcp_rx_buf_max_size = 0x200000;
  sockCfg.sb_efficiency = 8;
  socketInitialize(&sockCfg);

  nxlinkStdio();
}
} // namespace

int main(int argc, char* argv[])
{
  BringUpServices();

  mkdir(KODI_ROOT, 0777);
  chdir(KODI_ROOT);
  setenv("HOME", KODI_HOME, 1);
  setenv("KODI_BIN_HOME", KODI_HOME, 1);
  setlocale(LC_NUMERIC, "C");

  CAppParamParser appParamParser;
  appParamParser.Parse(argv, argc);

  CAppEnvironment::SetUp(appParamParser.GetAppParams());
  int status = XBMC_Run(true);
  CAppEnvironment::TearDown();

  socketExit();
  return status;
}
