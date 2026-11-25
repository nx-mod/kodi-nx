/*
 *  Copyright (C) 2026 Team Kodi
 *  This file is part of Kodi - https://kodi.tv
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 *  See LICENSES/README.md for more information.
 */

#include "NetworkSwitch.h"

#include <cstring>

#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <switch.h>

namespace
{
std::string Ipv4ToString(u32 addr)
{
  struct in_addr in;
  in.s_addr = addr;
  char buf[INET_ADDRSTRLEN] = {};
  inet_ntop(AF_INET, &in, buf, sizeof(buf));
  return buf;
}

struct IpConfig
{
  bool ok = false;
  u32 ip = 0, mask = 0, gateway = 0, dns1 = 0, dns2 = 0;
};

IpConfig QueryIpConfig()
{
  IpConfig cfg;
  cfg.ok = R_SUCCEEDED(nifmGetCurrentIpConfigInfo(&cfg.ip, &cfg.mask, &cfg.gateway, &cfg.dns1,
                                                  &cfg.dns2));
  return cfg;
}
} // namespace

std::unique_ptr<CNetworkBase> CNetworkBase::GetNetwork()
{
  return std::make_unique<CNetworkSwitch>();
}

bool CNetworkInterfaceSwitch::IsConnected() const
{
  // The console's LAN has no internet, so nifm's internet status never reports Connected;
  // having an IPv4 address is what Kodi needs for local network sources.
  IpConfig cfg = QueryIpConfig();
  return cfg.ok && cfg.ip != 0;
}

void CNetworkInterfaceSwitch::GetMacAddressRaw(char rawMac[6]) const
{
  std::memset(rawMac, 0, 6);
}

std::string CNetworkInterfaceSwitch::GetCurrentIPAddress() const
{
  IpConfig cfg = QueryIpConfig();
  return cfg.ok ? Ipv4ToString(cfg.ip) : "";
}

std::string CNetworkInterfaceSwitch::GetCurrentNetmask() const
{
  IpConfig cfg = QueryIpConfig();
  return cfg.ok ? Ipv4ToString(cfg.mask) : "";
}

std::string CNetworkInterfaceSwitch::GetCurrentDefaultGateway() const
{
  IpConfig cfg = QueryIpConfig();
  return cfg.ok ? Ipv4ToString(cfg.gateway) : "";
}

CNetworkSwitch::CNetworkSwitch()
{
  m_nifm = R_SUCCEEDED(nifmInitialize(NifmServiceType_User));
  m_interfaces.push_back(&m_interface);
}

CNetworkSwitch::~CNetworkSwitch()
{
  m_interfaces.clear();
  if (m_nifm)
    nifmExit();
}

std::vector<std::string> CNetworkSwitch::GetNameServers()
{
  std::vector<std::string> servers;
  IpConfig cfg = QueryIpConfig();
  if (cfg.ok)
  {
    if (cfg.dns1)
      servers.push_back(Ipv4ToString(cfg.dns1));
    if (cfg.dns2)
      servers.push_back(Ipv4ToString(cfg.dns2));
  }
  return servers;
}
