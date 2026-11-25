/*
 *  Copyright (C) 2026 Team Kodi
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
// Force-included into every Kodi source on Switch (see cmake/scripts/switch/ArchSetup.cmake).
// Fills small gaps between newlib and what Kodi expects from glibc. Keep it minimal.
#pragma once

#include <sys/types.h>
// newlib's <sys/types.h> does "#define quad quad_t" under __BSD_VISIBLE; Kodi has members named quad.
#undef quad

#include <alloca.h>
#include <stdio.h>
#include <time.h>

// newlib only exports the reserved-name variant. (Not getline: a macro would break std::getline.)
#define getdelim __getdelim

#ifdef __cplusplus
extern "C" {
#endif
// Inverse of gmtime(); implemented in xbmc/platform/switch/PosixCompat.cpp.
time_t timegm(struct tm* tm);
#ifdef __cplusplus
}
#endif
