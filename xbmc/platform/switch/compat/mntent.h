/*
 *  Copyright (C) 2026 Team Kodi
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
// Switch 2 compat: <mntent.h>. There is no mount table; every lookup finds nothing.
#pragma once
#include <stdio.h>
#ifdef __cplusplus
extern "C" {
#endif
struct mntent
{
  char* mnt_fsname;
  char* mnt_dir;
  char* mnt_type;
  char* mnt_opts;
  int mnt_freq;
  int mnt_passno;
};
static inline FILE* setmntent(const char*, const char*) { return 0; }
static inline struct mntent* getmntent(FILE*) { return 0; }
static inline int endmntent(FILE*) { return 1; }
#ifdef __cplusplus
}
#endif
