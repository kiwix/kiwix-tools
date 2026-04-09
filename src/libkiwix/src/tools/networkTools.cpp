/*
 * Copyright 2012 Emmanuel Engelhart <kelson@kiwix.org>
 * Copyright 2021 Nikhil Tanwar <2002nikhiltanwar@gmail.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU  General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
 * MA 02110-1301, USA.
 */

#include "tools.h"
#include "stringTools.h"
#include <tools/networkTools.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <curl/curl.h>

#include <sstream>
#include <iostream>
#include <stdexcept>

#ifdef _WIN32
#include <iphlpapi.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <iostream>
#else
#include <unistd.h>
#include <arpa/inet.h>
#include <ifaddrs.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <net/if.h>
#include <netdb.h>
#endif

#ifdef __HAIKU__
#include <sys/sockio.h>
#endif

namespace kiwix
{

namespace
{

size_t write_callback_to_iss(char* ptr, size_t size, size_t nmemb, void* userdata)
{
  auto str = static_cast<std::stringstream*>(userdata);
  str->write(ptr, nmemb);
  return nmemb;
}

void updatePublicIpAddress(IpAddress& publicIpAddr, const IpAddress& interfaceIpAddr)
{
  if (publicIpAddr.addr.empty())  publicIpAddr.addr  = interfaceIpAddr.addr;
  if (publicIpAddr.addr6.empty()) publicIpAddr.addr6 = interfaceIpAddr.addr6;
}

} // unnamed namespace

std::string download(const std::string& url) {
  auto curl = curl_easy_init();
  std::stringstream ss;
  curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
  curl_easy_setopt(curl, CURLOPT_HTTPGET, 1L);
  curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, &write_callback_to_iss);
  curl_easy_setopt(curl, CURLOPT_WRITEDATA, &ss);
  auto res = curl_easy_perform(curl);
  if (res != CURLE_OK) {
    curl_easy_cleanup(curl);
    throw std::runtime_error("Cannot perform request");
  }
  long response_code;
  curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &response_code);
  curl_easy_cleanup(curl);
  if (response_code != 200) {
    throw std::runtime_error("Invalid return code from server");
  }
  return ss.str();
}

namespace
{

#ifdef _WIN32

std::map<std::string, IpAddress> getNetworkInterfacesWin() {
  std::map<std::string, IpAddress> interfaces;

  const int working_buffer_size = 15000;
  const int max_tries = 3;

  ULONG flags = GAA_FLAG_INCLUDE_PREFIX;

  // default to unspecified address family (both)
  ULONG family = AF_UNSPEC;

  ULONG outBufLen = working_buffer_size;
  ULONG Iterations = 0;
  DWORD dwRetVal = 0;
  PIP_ADAPTER_ADDRESSES interfacesHead = NULL;

  // Successively allocate the required memory until GetAdaptersAddresses does not
  // results in ERROR_BUFFER_OVERFLOW for a maximum of max_tries
  do{
    if (interfacesHead) {
      free(interfacesHead);
      interfacesHead = NULL;
    }
    interfacesHead = (IP_ADAPTER_ADDRESSES *) malloc(outBufLen);
    if (interfacesHead == NULL) {
      std::cerr << "Memory allocation failed for IP_ADAPTER_ADDRESSES struct" << std::endl;
      return interfaces;
    }

    dwRetVal = GetAdaptersAddresses(family, flags, NULL, interfacesHead, &outBufLen);
    Iterations++;
  } while ((dwRetVal == ERROR_BUFFER_OVERFLOW) && (Iterations < max_tries));

  if (dwRetVal == NO_ERROR) {
    PIP_ADAPTER_UNICAST_ADDRESS pUnicast = NULL;
    unsigned int i = 0;
    for (PIP_ADAPTER_ADDRESSES temp = interfacesHead; temp != NULL;
    temp = temp->Next) {
      pUnicast = temp->FirstUnicastAddress;
            if (pUnicast != NULL) {
                for (i = 0; pUnicast != NULL; i++){
                  if (pUnicast->Address.lpSockaddr->sa_family == AF_INET)
                  {
                      sockaddr_in *si = (sockaddr_in *)(pUnicast->Address.lpSockaddr);
                      char host[INET_ADDRSTRLEN]={0};
                      inet_ntop(AF_INET, &(si->sin_addr), host, sizeof(host));
                      interfaces[temp->AdapterName].addr=host;
                  }
                  else if (pUnicast->Address.lpSockaddr->sa_family == AF_INET6)
                  {
                      sockaddr_in6 *si = (sockaddr_in6 *)(pUnicast->Address.lpSockaddr);
                      char host[INET6_ADDRSTRLEN]={0};
                      inet_ntop(AF_INET6, &(si->sin6_addr), host, sizeof(host));
                      if (!IN6_IS_ADDR_LINKLOCAL(&(si->sin6_addr)))
                        interfaces[temp->AdapterName].addr6=host;
                  }
                  pUnicast = pUnicast->Next;
                }
            }
    }
  } else {
    std::cerr << "Call to GetAdaptersAddresses failed with error: "<< dwRetVal << std::endl;
  }

  if (interfacesHead) free(interfacesHead);

  return interfaces;
}

#else

std::map<std::string, IpAddress> getNetworkInterfacesPosix() {
  std::map<std::string, IpAddress> interfaces;

  struct ifaddrs *interfacesHead;
  if (getifaddrs(&interfacesHead) == -1) {
      perror("getifaddrs");
  }

  for (ifaddrs *temp = interfacesHead; temp != NULL; temp = temp->ifa_next) {
      if (temp->ifa_addr == NULL) continue;

      if (temp->ifa_addr->sa_family == AF_INET) {
          sockaddr_in *si = (sockaddr_in *)(temp->ifa_addr);
          char host[INET_ADDRSTRLEN] = {0};
          inet_ntop(AF_INET, &(si->sin_addr), host, sizeof(host));
          interfaces[temp->ifa_name].addr=host;
      } else if (temp->ifa_addr->sa_family == AF_INET6) {
          sockaddr_in6 *si = (sockaddr_in6 *)(temp->ifa_addr);
          char host[INET6_ADDRSTRLEN] = {0};
          inet_ntop(AF_INET6, &(si->sin6_addr), host, sizeof(host));
          if (!IN6_IS_ADDR_LINKLOCAL(&(si->sin6_addr)))
            interfaces[temp->ifa_name].addr6=host;
      }
  }

  freeifaddrs(interfacesHead);

  return interfaces;
}

#endif

} // unnamed namespace

std::map<std::string, IpAddress> getNetworkInterfacesIPv4Or6() {
#ifdef _WIN32
  return getNetworkInterfacesWin();
#else
  return getNetworkInterfacesPosix();
#endif
}

std::map<std::string, std::string> getNetworkInterfaces() {
  std::map<std::string, std::string> result;
  for ( const auto& kv : getNetworkInterfacesIPv4Or6() ) {
      const std::string& interfaceName = kv.first;
      const auto& ipAddresses = kv.second;
      if ( !ipAddresses.addr.empty() ) {
        result[interfaceName] = ipAddresses.addr;
      }
  }
  return result;
}

IpAddress getBestPublicIps() {
  IpAddress bestPublicIps;
  std::map<std::string, IpAddress> interfaces = getNetworkInterfacesIPv4Or6();
#ifndef _WIN32
  const char* const prioritizedNames[] = { "eth0", "eth1", "wlan0", "wlan1", "en0", "en1" };
  for (const auto& name : prioritizedNames) {
    const auto it = interfaces.find(name);
    if (it != interfaces.end()) {
      updatePublicIpAddress(bestPublicIps, it->second);
    }
  }
#endif
  const char* const v4prefixes[] = { "192.168", "172.16", "10.211", "10.0" };
  for (const auto& prefix : v4prefixes) {
    for (const auto& kv : interfaces) {
      const auto& interfaceIps = kv.second;
      if (kiwix::startsWith(interfaceIps.addr, prefix)) {
        updatePublicIpAddress(bestPublicIps, interfaceIps);
      }
    }
  }

  updatePublicIpAddress(bestPublicIps, {"127.0.0.1", "::1"});

  return bestPublicIps;
}

std::string getBestPublicIp()
{
  return getBestPublicIps().addr;
}

} // namespace kiwix
