/*
 *  Copyright (C) 2026 Team Kodi
 *  This file is part of Kodi - https://kodi.tv
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 *  See LICENSES/README.md for more information.
 */

#include <ctime>
#include <condition_variable>
#include <deque>
#include <mutex>
#include <pthread.h>
#include <switch.h>
#include <cerrno>
#include <cstdio>
#include <cstring>
#include <sys/stat.h>
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

extern "C" int __real_pthread_create(pthread_t*, const pthread_attr_t*, void* (*)(void*), void*);

extern "C" int __wrap_pthread_create(pthread_t* thread,
                                     const pthread_attr_t* attr,
                                     void* (*start)(void*),
                                     void* arg)
{
  if (attr)
    return __real_pthread_create(thread, attr, start, arg);

  pthread_attr_t big;
  pthread_attr_init(&big);
  pthread_attr_setstacksize(&big, 1u << 20);
  const int rc = __real_pthread_create(thread, &big, start, arg);
  pthread_attr_destroy(&big);
  return rc;
}

namespace
{
std::mutex g_reapMutex;
std::condition_variable g_reapCv;
std::deque<pthread_t> g_reapQueue;
bool g_reaperRunning = false;

void* Reaper(void*)
{
  for (;;)
  {
    pthread_t t;
    {
      std::unique_lock<std::mutex> lock(g_reapMutex);
      g_reapCv.wait(lock, [] { return !g_reapQueue.empty(); });
      t = g_reapQueue.front();
      g_reapQueue.pop_front();
    }
    pthread_join(t, nullptr);
  }
  return nullptr;
}

void StartReaper()
{
  pthread_attr_t attr;
  pthread_attr_init(&attr);
  pthread_attr_setstacksize(&attr, 64 * 1024);
  pthread_t reaper;
  const int rc = __real_pthread_create(&reaper, &attr, Reaper, nullptr);
  pthread_attr_destroy(&attr);
  g_reaperRunning = rc == 0;
}
} // namespace

extern "C" int __wrap_pthread_detach(pthread_t thread)
{
  static std::once_flag once;
  std::call_once(once, StartReaper);
  if (!g_reaperRunning)
    return 0;
  {
    std::lock_guard<std::mutex> lock(g_reapMutex);
    g_reapQueue.push_back(thread);
  }
  g_reapCv.notify_one();
  return 0;
}

extern "C" int pause()
{
  for (;;)
    svcSleepThread(1000000000ULL);
  return -1;
}

extern "C" int fstatat(int dirfd, const char* path, struct stat* st, int flags)
{
  (void)dirfd;
  (void)flags;
  return stat(path, st);
}

extern "C" void __real_exit(int);
extern "C" int main(int, char**);

extern "C" void __wrap_exit(int code)
{
  FILE* mark = fopen("sdmc:/switch/kodi/boot.txt", "a");
  if (mark)
  {
    fprintf(mark, "exit-called %d caller %p main %p\n", code, __builtin_return_address(0), reinterpret_cast<void*>(&main));
    fclose(mark);
  }
  __real_exit(code);
}

extern "C" void __real__Py_FatalErrorFunc(const char*, const char*);

extern "C" void __wrap__Py_FatalErrorFunc(const char* func, const char* message)
{
  FILE* mark = fopen("sdmc:/switch/kodi/boot.txt", "a");
  if (mark)
  {
    fprintf(mark, "python-fatal %s: %s\n", func ? func : "?", message ? message : "?");
    fclose(mark);
  }
  __real__Py_FatalErrorFunc(func, message);
}

extern "C" int __real__Py_write_noraise(int, const void*, size_t);

extern "C" int __wrap__Py_write_noraise(int fd, const void* buf, size_t size)
{
  if (fd == 2 && size && size < 1024)
  {
    FILE* mark = fopen("sdmc:/switch/kodi/boot.txt", "a");
    if (mark)
    {
      fwrite("python-write ", 1, 13, mark);
      fwrite(buf, 1, size, mark);
      fputc('\n', mark);
      fclose(mark);
    }
  }
  return __real__Py_write_noraise(fd, buf, size);
}
