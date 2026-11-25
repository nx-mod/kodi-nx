/*
 *  Copyright (C) 2026 Team Kodi
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
// Switch 2 compat: <sys/mman.h>. newlib has no mmap; these fail with ENOSYS so Kodi's
// mmap users (shared-memory and DMA buffers, unused on this platform) take their error paths.
#pragma once
#include <errno.h>
#include <stddef.h>
#include <sys/types.h>
#define PROT_NONE 0x0
#define PROT_READ 0x1
#define PROT_WRITE 0x2
#define PROT_EXEC 0x4
#define MAP_SHARED 0x01
#define MAP_PRIVATE 0x02
#define MAP_FIXED 0x10
#define MAP_ANONYMOUS 0x20
#define MAP_ANON MAP_ANONYMOUS
#define MAP_FAILED ((void*)-1)
#ifdef __cplusplus
extern "C" {
#endif
static inline void* mmap(void*, size_t, int, int, int, off_t)
{
  errno = ENOSYS;
  return MAP_FAILED;
}
static inline int munmap(void*, size_t)
{
  errno = ENOSYS;
  return -1;
}
#ifdef __cplusplus
}
#endif
