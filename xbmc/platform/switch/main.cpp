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

#include <cstdio>
#include <cstdlib>
#include <locale.h>
#include <sys/stat.h>
#include <unistd.h>

#include <switch.h>

namespace
{
constexpr const char* KODI_ROOT = "sdmc:/switch/kodi";
constexpr const char* KODI_HOME = "/switch/kodi";

void BootMark(const char* what, int value = 0)
{
  constexpr const char* path = "sdmc:/switch/kodi/boot.txt";
  fsdevCreateFile(path, 0, 0);

  FsFileSystem* fs = nullptr;
  char fsPath[FS_MAX_PATH] = {};
  if (R_FAILED(fsdevTranslatePath(path, &fs, fsPath)) || !fs)
    return;

  FsFile file;
  if (R_FAILED(fsFsOpenFile(fs, fsPath, FsOpenMode_Write | FsOpenMode_Append, &file)))
    return;

  s64 size = 0;
  fsFileGetSize(&file, &size);
  char line[128];
  const int len = snprintf(line, sizeof(line), "%s %d\n", what, value);
  fsFileWrite(&file, size, line, static_cast<u64>(len), 0);
  fsFileClose(&file);
}

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
  fsdevMountSdmc();

  mkdir(KODI_ROOT, 0777);
  BootMark("main-entered");

  BringUpServices();
  BootMark("services-up");

  chdir(KODI_ROOT);
  setenv("HOME", KODI_HOME, 1);
  setenv("KODI_BIN_HOME", KODI_HOME, 1);
  setenv("CURL_CA_BUNDLE", "sdmc:/switch/kodi/system/certs/cacert.pem", 1);
  setlocale(LC_NUMERIC, "C");
  BootMark("environment-ready");

  CAppParamParser appParamParser;
  appParamParser.Parse(argv, argc);
  BootMark("params-parsed");

  CAppEnvironment::SetUp(appParamParser.GetAppParams());
  BootMark("environment-set-up");

  int status = XBMC_Run(true);
  BootMark("xbmc-run-returned", status);

  CAppEnvironment::TearDown();
  BootMark("torn-down");

  socketExit();
  return status;
}
