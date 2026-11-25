/*
 *  Copyright (C) 2026 Team Kodi
 *  This file is part of Kodi - https://kodi.tv
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 *  See LICENSES/README.md for more information.
 */

#pragma once

#include "network/Network.h"

#include <string>
#include <vector>

// The console has one network interface; nifm reports its state and IPv4 configuration.
class CNetworkInterfaceSwitch : public CNetworkInterface
{
public:
  bool IsEnabled() const override { return true; }
  bool IsConnected() const override;
  std::string GetMacAddress() const override { return "00:00:00:00:00:00"; }
  void GetMacAddressRaw(char rawMac[6]) const override;
  bool GetHostMacAddress(unsigned long host, std::string& mac) const override { return false; }
  std::string GetCurrentIPAddress() const override;
  std::string GetCurrentNetmask() const override;
  std::string GetCurrentDefaultGateway() const override;
};

class CNetworkSwitch : public CNetworkBase
{
public:
  CNetworkSwitch();
  ~CNetworkSwitch() override;

  std::vector<CNetworkInterface*>& GetInterfaceList() override { return m_interfaces; }
  bool PingHost(unsigned long host, unsigned int timeout_ms = 2000) override { return false; }
  std::vector<std::string> GetNameServers() override;

private:
  CNetworkInterfaceSwitch m_interface;
  std::vector<CNetworkInterface*> m_interfaces;
  bool m_nifm = false;
};
