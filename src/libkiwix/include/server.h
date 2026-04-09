/*
 * Copyright 2019 Matthieu Gautier <mgautier@kymeria.fr>
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

#ifndef KIWIX_SERVER_H
#define KIWIX_SERVER_H

#include <string>
#include <memory>
#include "tools.h"

namespace kiwix
{
  class Library;
  class NameMapper;
  class InternalServer;

  class Server {
     public:
       /**
        * The default constructor.
        *
        * @param library The library to serve.
        */
       Server(std::shared_ptr<Library> library, std::shared_ptr<NameMapper> nameMapper=nullptr);

       virtual ~Server();

       /**
        * Start serving the content.
        */
       bool start();

       /**
        * Stop the daemon.
        */
       void stop();

       /**
        * Set the path of the root URL served by this server instance.
        */
       void setRoot(const std::string& root);

       /**
        * Set the IP address on which to listen for incoming connections.
        *
        * Specifying a non-empty IP address requires that the IpMode is
        * [set](@ref setIpMode()) to `IpMode::AUTO` (which is the default).
        * Otherwise, [starting](@ref start()) the server will fail.
        */
       void setAddress(const std::string& addr);

       /**
        * Set the port on which to listen for incoming connections.
        *
        * Default port is 80, but using it requires special privileges.
        */
       void setPort(int port) { m_port = port; }

       void setNbThreads(int threads) { m_nbThreads = threads; }
       void setMultiZimSearchLimit(unsigned int limit) { m_multizimSearchLimit = limit; }
       void setIpConnectionLimit(int limit) { m_ipConnectionLimit = limit; }
       void setVerbose(bool verbose) { m_verbose = verbose; }
       void setIndexTemplateString(const std::string& indexTemplateString) { m_indexTemplateString = indexTemplateString; }
       void setTaskbar(bool withTaskbar, bool withLibraryButton)
        { m_withTaskbar = withTaskbar; m_withLibraryButton = withLibraryButton; }
       void setBlockExternalLinks(bool blockExternalLinks)
        { m_blockExternalLinks = blockExternalLinks; }
       void setCatalogOnlyMode(bool enable) { m_catalogOnlyMode = enable; }
       void setContentServerUrl(std::string url) { m_contentServerUrl = url; }

       /**
        * Listen for incoming connections on all IP addresses of the specified
        * IP protocol family.
        */
       void setIpMode(IpMode mode) { m_ipMode = mode; }

       /**
        * Get the port on which the server listens for incoming connections
        */
       int getPort() const;

       /**
        * Get the IPv4 and/or IPv6 address(es) on which the server can be
        * contacted.
        *
        * The server may actually be listening on other IP addresses as well
        * (see `setIpMode()`).  The IP address(es) returned by this method
        * represent the best-guess public IP address(es) accessible by the
        * broadest set of clients.
        */
       IpAddress getAddress() const;

       /**
        * Get the effective IpMode used by this server.
        *
        * The returned value may be different from the one configured via
        * `setIpMode()`, since the server is expected to adjust to the
        * constraints of the environment (e.g. `IpMode::ALL` can be converted
        * to `IpMode::IPV4` on a system that does not support IPv6).
        */
       IpMode getIpMode() const;

       /**
        * Get the list of HTTP URLs through which the server can be contacted.
        *
        * Each URL is composed of an IP address and optional port and includes
        * the [root](@ref setRoot()) path component.
        *
        * In the current implementation at most 2 URLs may be returned - one
        * for IPv4 and another for IPv6 protocol (whichever is available), as
        * returned by `getAddress()`. Note, however, that the server may be
        * also accessible via other IP-addresses/URLs (see `setIpMode()`).
        */
       std::vector<std::string> getServerAccessUrls() const;

     protected:
       std::shared_ptr<Library> mp_library;
       std::shared_ptr<NameMapper> mp_nameMapper;
       std::string m_root = "";
       IpAddress m_addr;
       std::string m_indexTemplateString = "";
       int m_port = 80;
       int m_nbThreads = 1;
       unsigned int m_multizimSearchLimit = 0;
       bool m_verbose = false;
       bool m_withTaskbar = true;
       bool m_withLibraryButton = true;
       bool m_blockExternalLinks = false;
       IpMode m_ipMode = IpMode::AUTO;
       int m_ipConnectionLimit = 0;
       bool m_catalogOnlyMode = false;
       std::string m_contentServerUrl;
       std::unique_ptr<InternalServer> mp_server;
  };
}

#endif
