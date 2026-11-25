/*
 *  Copyright (C) 2026 Team Kodi
 *  This file is part of Kodi - https://kodi.tv
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 *  See LICENSES/README.md for more information.
 */

#include <ctime>
#include <cerrno>
#include <cstdio>
#include <cstring>
#include <sys/utsname.h>
#include <sys/resource.h>
#include <unistd.h>

namespace
{
long long DaysFromCivil(long long year, unsigned month, unsigned day)
{
  year -= month <= 2;
  const long long era{(year >= 0 ? year : year - 399) / 400};
  const unsigned yoe{static_cast<unsigned>(year - era * 400)};
  const unsigned doy{(153 * (month + (month > 2 ? -3 : 9)) + 2) / 5 + day - 1};
  const unsigned doe{yoe * 365 + yoe / 4 - yoe / 100 + doy};
  return era * 146097 + static_cast<long long>(doe) - 719468;
}
} // namespace

extern "C" time_t timegm(struct tm* tm)
{
  if (!tm)
    return static_cast<time_t>(-1);

  long long year = 1900LL + tm->tm_year + tm->tm_mon / 12;
  int mon = tm->tm_mon % 12;
  if (mon < 0)
  {
    mon += 12;
    year -= 1;
  }

  const long long days =
      DaysFromCivil(year, static_cast<unsigned>(mon + 1), 1) + (tm->tm_mday - 1);
  const long long secs =
      days * 86400LL + tm->tm_hour * 3600LL + tm->tm_min * 60LL + tm->tm_sec;
  const time_t t = static_cast<time_t>(secs);

  struct tm norm;
  if (gmtime_r(&t, &norm))
    *tm = norm;
  return t;
}

extern "C" uid_t geteuid()
{
  return 0;
}

extern "C" FILE* popen(const char*, const char*)
{
  errno = ENOSYS;
  return nullptr;
}

extern "C" int pclose(FILE*)
{
  errno = ENOSYS;
  return -1;
}

extern "C" int getrusage(int, struct rusage* usage)
{
  if (!usage)
  {
    errno = EINVAL;
    return -1;
  }
  *usage = {};
  return 0;
}

extern "C" int uname(struct utsname* info)
{
  if (!info)
  {
    errno = EFAULT;
    return -1;
  }
  std::strncpy(info->sysname, "Horizon", sizeof(info->sysname));
  std::strncpy(info->nodename, "switch", sizeof(info->nodename));
  std::strncpy(info->release, "23.0.1", sizeof(info->release));
  std::strncpy(info->version, "#1", sizeof(info->version));
  std::strncpy(info->machine, "aarch64", sizeof(info->machine));
  return 0;
}

extern "C" int execvp(const char*, char* const[])
{
  errno = ENOSYS;
  return -1;
}

extern "C" pid_t waitpid(pid_t, int*, int)
{
  errno = ECHILD;
  return -1;
}
