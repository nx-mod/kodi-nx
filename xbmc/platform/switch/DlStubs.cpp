/*
 *  Copyright (C) 2026 Team Kodi
 *  This file is part of Kodi - https://kodi.tv
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 *  See LICENSES/README.md for more information.
 */

#include <cstring>
#include <dlfcn.h>

extern "C" int vfs_tmdbm_ADDON_Create(void*);
extern "C" const char* vfs_tmdbm_ADDON_GetTypeVersion(int);
extern "C" const char* vfs_tmdbm_ADDON_GetTypeMinVersion(int);

namespace
{
int g_vfsTmdbmHandle;
}

extern "C" void* dlopen(const char* path, int)
{
  if (path && std::strstr(path, "vfs.tmdbm"))
    return &g_vfsTmdbmHandle;
  return nullptr;
}

extern "C" void* dlsym(void* handle, const char* name)
{
  if (handle != &g_vfsTmdbmHandle || !name)
    return nullptr;

  if (std::strcmp(name, "ADDON_Create") == 0)
    return reinterpret_cast<void*>(&vfs_tmdbm_ADDON_Create);
  if (std::strcmp(name, "ADDON_GetTypeVersion") == 0)
    return reinterpret_cast<void*>(&vfs_tmdbm_ADDON_GetTypeVersion);
  if (std::strcmp(name, "ADDON_GetTypeMinVersion") == 0)
    return reinterpret_cast<void*>(&vfs_tmdbm_ADDON_GetTypeMinVersion);

  return nullptr;
}

extern "C" int dlclose(void*)
{
  return 0;
}

extern "C" char* dlerror()
{
  return nullptr;
}

extern "C" void init_emu_environ() {}
extern "C" void update_emu_environ() {}
extern "C" void cleanup_emu_environ() {}
