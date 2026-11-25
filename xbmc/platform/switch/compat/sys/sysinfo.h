/*
 *  Copyright (C) 2026 Team Kodi
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
// Switch 2 compat: Linux <sys/sysinfo.h>. sysinfo() reports process memory from svcGetInfo.
#pragma once
#ifdef __cplusplus
extern "C" {
#endif
struct sysinfo
{
  long uptime;
  unsigned long loads[3];
  unsigned long totalram;
  unsigned long freeram;
  unsigned long sharedram;
  unsigned long bufferram;
  unsigned long totalswap;
  unsigned long freeswap;
  unsigned short procs;
  unsigned long totalhigh;
  unsigned long freehigh;
  unsigned int mem_unit;
};
int sysinfo(struct sysinfo* info);
int get_nprocs(void);
int get_nprocs_conf(void);
#ifdef __cplusplus
}
#endif
