/*
 *  Copyright (C) 2026 Team Kodi
 *  This file is part of Kodi - https://kodi.tv
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 *  See LICENSES/README.md for more information.
 */

#include <dlfcn.h>

extern "C" void* dlopen(const char*, int)
{
  return nullptr;
}

extern "C" void* dlsym(void*, const char*)
{
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
