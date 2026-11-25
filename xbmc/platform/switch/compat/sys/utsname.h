/*
 *  Copyright (C) 2026 Team Kodi
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
// Switch 2 compat: <sys/utsname.h>.
#pragma once
#ifdef __cplusplus
extern "C" {
#endif
struct utsname
{
  char sysname[65];
  char nodename[65];
  char release[65];
  char version[65];
  char machine[65];
};
int uname(struct utsname* name);
#ifdef __cplusplus
}
#endif
