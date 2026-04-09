/*
 * Copyright 2018 Matthieu Gautier <mgautier@kymeria.fr>
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

#include "downloader.h"
#include "tools.h"
#include "tools/pathTools.h"
#include "tools/stringTools.h"

#include <algorithm>
#include <thread>
#include <chrono>

#include <iostream>

#include "aria2.h"
#include "xmlrpc.h"
#include "tools/otherTools.h"
#include <pugixml.hpp>

namespace kiwix
{

void Download::updateStatus(bool follow)
{
  if (m_status == Download::K_REMOVED)
    return;
  static std::vector<std::string> statusKey = {"status", "files", "totalLength",
                                               "completedLength", "followedBy",
                                               "downloadSpeed", "verifiedLength"};
  std::string strStatus;
  if(follow && !m_followedBy.empty()) {
    strStatus = mp_aria->tellStatus(m_followedBy, statusKey);
  } else {
    strStatus = mp_aria->tellStatus(m_did, statusKey);
  }
//  std::cout << strStatus << std::endl;
  MethodResponse response(strStatus);
  if (response.isFault()) {
    m_status = Download::K_UNKNOWN;
    return;
  }
  auto structNode = response.getParams().getParam(0).getValue().getStruct();
  auto _status = structNode.getMember("status").getValue().getAsS();
  auto status = _status == "active" ? Download::K_ACTIVE
              : _status == "waiting" ? Download::K_WAITING
              : _status == "paused" ? Download::K_PAUSED
              : _status == "error" ? Download::K_ERROR
              : _status == "complete" ? Download::K_COMPLETE
              : _status == "removed" ? Download::K_REMOVED
              : Download::K_UNKNOWN;
  if (status == K_COMPLETE) {
    try {
      auto followedByMember = structNode.getMember("followedBy");
      m_followedBy = followedByMember.getValue().getArray().getValue(0).getAsS();
      if (follow) {
        status = K_ACTIVE;
        updateStatus(true);
        return;
      }
    } catch (InvalidRPCNode& e) { }
  }
  m_status = status;
  m_totalLength = extractFromString<uint64_t>(structNode.getMember("totalLength").getValue().getAsS());
  m_completedLength = extractFromString<uint64_t>(structNode.getMember("completedLength").getValue().getAsS());
  m_downloadSpeed = extractFromString<uint64_t>(structNode.getMember("downloadSpeed").getValue().getAsS());
  try {
    auto verifiedLengthValue = structNode.getMember("verifiedLength").getValue();
    m_verifiedLength = extractFromString<uint64_t>(verifiedLengthValue.getAsS());
  } catch (InvalidRPCNode& e) { m_verifiedLength = 0; }
  auto filesMember = structNode.getMember("files");
  auto fileStruct = filesMember.getValue().getArray().getValue(0).getStruct();
  m_path = fileStruct.getMember("path").getValue().getAsS();
  auto urisArray = fileStruct.getMember("uris").getValue().getArray();
  int index = 0;
  m_uris.clear();
  while(true) {
    try {
      auto uriNode = urisArray.getValue(index++).getStruct().getMember("uri");
      m_uris.push_back(uriNode.getValue().getAsS());
    } catch(InvalidRPCNode& e) { break; }
  }
}

void Download::resumeDownload()
{
    if (!m_followedBy.empty())
        mp_aria->unpause(m_followedBy);
    else
        mp_aria->unpause(m_did);
    updateStatus(true);
}

void Download::pauseDownload()
{
    if (!m_followedBy.empty())
        mp_aria->pause(m_followedBy);
    else
        mp_aria->pause(m_did);
    updateStatus(true);
}

void Download::cancelDownload()
{
    if (!m_followedBy.empty())
        mp_aria->remove(m_followedBy);
    else
        mp_aria->remove(m_did);
    m_status = Download::K_REMOVED;
}

/* Constructor */
Downloader::Downloader(std::string sessionFileDir) :
  mp_aria(new Aria2(sessionFileDir))
{
  try {
    for (auto gid : mp_aria->tellWaiting()) {
      m_knownDownloads[gid] = std::unique_ptr<Download>(new Download(mp_aria, gid));
      m_knownDownloads[gid]->updateStatus(false);
    }
  } catch (std::exception& e) {
    std::cerr << "aria2 tellWaiting failed : " << e.what() << std::endl;
  }
  try {
    for (auto gid : mp_aria->tellActive()) {
      if( m_knownDownloads.find(gid) == m_knownDownloads.end()) {
        m_knownDownloads[gid] = std::unique_ptr<Download>(new Download(mp_aria, gid));
        m_knownDownloads[gid]->updateStatus(false);
      }
    }
  } catch (std::exception& e) {
    std::cerr << "aria2 tellActive failed : " << e.what() << std::endl;
  }
}

/* Destructor */
Downloader::~Downloader()
{
  close();
}

void Downloader::close()
{
  if ( mp_aria ) {
    try {
      mp_aria->close();
    } catch (const std::exception& err) {
      std::cerr << "ERROR: Failed to save the downloader state: "
                << err.what() << std::endl;
    }
    mp_aria.reset();
  }
}

std::vector<std::string> Downloader::getDownloadIds() const {
  std::unique_lock<std::mutex> lock(m_lock);
  std::vector<std::string> ret;
  for(auto& p:m_knownDownloads) {
    ret.push_back(p.first);
  }
  return ret;
}

namespace
{

bool downloadCanBeReused(const Download& d,
                         const std::string& uri,
                         const Downloader::Options& /*options*/)
{
  const auto& uris = d.getUris();
  const bool sameURI = std::find(uris.begin(), uris.end(), uri) != uris.end();

  if ( !sameURI )
    return false;

  switch ( d.getStatus() ) {
  case Download::K_ERROR:
  case Download::K_UNKNOWN:
  case Download::K_REMOVED:
    return false;

  case Download::K_ACTIVE:
  case Download::K_WAITING:
  case Download::K_PAUSED:
    return true; // XXX: what if options are different?

  case Download::K_COMPLETE:
    return fileExists(d.getPath()); // XXX: what if options are different?
  }

  return false;
}

} // unnamed namespace

std::shared_ptr<Download> Downloader::startDownload(const std::string& uri, const std::string& downloadDir, Options options)
{
  std::unique_lock<std::mutex> lock(m_lock);
  options.erase(std::remove_if(options.begin(), options.end(), [](const auto& option) {
    return option.first == "dir";
  }), options.end());
  options.push_back({"dir", downloadDir});
  for (auto& p: m_knownDownloads) {
    auto& d = p.second;
    if ( downloadCanBeReused(*d, uri, options) )
      return d;
  }
  std::vector<std::string> uris = {uri};
  auto gid = mp_aria->addUri(uris, options);
  m_knownDownloads[gid] = std::make_shared<Download>(mp_aria, gid);
  return m_knownDownloads[gid];
}

std::shared_ptr<Download> Downloader::getDownload(const std::string& did)
{
  std::unique_lock<std::mutex> lock(m_lock);
  try {
    return m_knownDownloads.at(did);
  } catch(std::exception& e) {
    for (auto gid : mp_aria->tellWaiting()) {
      if (gid == did) {
        m_knownDownloads[gid] = std::make_shared<Download>(mp_aria, gid);
        return m_knownDownloads[gid];
      }
    }
    for (auto gid : mp_aria->tellActive()) {
      if (gid == did) {
        m_knownDownloads[gid] = std::make_shared<Download>(mp_aria, gid);
        return m_knownDownloads[gid];
      }
    }
    throw;
  }
}

size_t Downloader::getNbDownload() const {
  std::unique_lock<std::mutex> lock(m_lock);
  return m_knownDownloads.size();
}

}
