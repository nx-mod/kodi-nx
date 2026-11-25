/*
 *  Copyright (C) 2026 Team Kodi
 *  This file is part of Kodi - https://kodi.tv
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 *  See LICENSES/README.md for more information.
 */

#include "SwitchStorageProvider.h"

#include "MediaSource.h"
#include "utils/StringUtils.h"

#include <sys/statvfs.h>

std::unique_ptr<IStorageProvider> IStorageProvider::CreateInstance()
{
  return std::make_unique<CSwitchStorageProvider>();
}

void CSwitchStorageProvider::GetLocalDrives(std::vector<CMediaSource>& localDrives)
{
  CMediaSource share;
  share.strPath = "/"; // the SD card (sdmc is the default device)
  share.strName = "SD card";
  share.m_ignore = true;
  share.m_iDriveType = SourceType::LOCAL;
  localDrives.push_back(share);
}

std::vector<std::string> CSwitchStorageProvider::GetDiskUsage()
{
  std::vector<std::string> result;
  struct statvfs fs;
  if (statvfs("/", &fs) == 0)
  {
    const double total = static_cast<double>(fs.f_blocks) * fs.f_frsize / (1024.0 * 1024 * 1024);
    const double avail = static_cast<double>(fs.f_bavail) * fs.f_frsize / (1024.0 * 1024 * 1024);
    result.push_back(StringUtils::Format("SD card: {:.1f} GB free of {:.1f} GB", avail, total));
  }
  return result;
}
