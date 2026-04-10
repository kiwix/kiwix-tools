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

#ifndef KIWIX_DOWNLOADER_H
#define KIWIX_DOWNLOADER_H

#include <string>
#include <vector>
#include <map>
#include <memory>
#include <stdexcept>
#include <mutex>

namespace kiwix
{

class Aria2;
struct DownloadedFile {
  DownloadedFile()
   : success(false) {}
  bool success;
  std::string path;
};

class AriaError : public std::runtime_error {
 public:
  AriaError(const std::string& message) : std::runtime_error(message) {}
};


/**
 * A representation of a current download.
 *
 * `Download` is not thread safe. User must care to not call method on a
 * same download from different threads.
 * However, it is safe to use different `Download`s from different threads.
 */

class Download {
 public:
  typedef enum { K_ACTIVE, K_WAITING, K_PAUSED, K_ERROR, K_COMPLETE, K_REMOVED, K_UNKNOWN } StatusResult;

  Download() :
    m_status(K_UNKNOWN) {}
  Download(std::shared_ptr<Aria2> p_aria, std::string did)
    : mp_aria(p_aria),
      m_status(K_UNKNOWN),
      m_did(did) {};

  /**
   * Update the status of the download.
   *
   * This call make an aria rpc call and is blocking.
   * Some download (started with a metalink) are in fact several downloads.
   * - A first one to download the metadlink.
   * - A second one to download the real file.
   *
   * If `follow` is true, updateStatus tries to detect that and tracks
   * the second download when the first one is finished.
   * By passing false to `follow`, `Download` will only track the first download.
   *
   * `getFoo` methods are based on the last statusUpdate.
   *
   * @param follow: Do we have to follow following downloads.
   */
  void updateStatus(bool follow);

  /**
   * Pause the download (and call updateStatus)
   */
  void pauseDownload();

  /**
   * Resume the download (and call updateStatus)
   */
  void resumeDownload();

  /**
   * Cancel the download.
   *
   * A canceled downlod cannot be resume and updateStatus does nothing.
   * However, you can still get information based on the last known information.
   */
  void cancelDownload();

  /*
   * Get the status of the download.
   */
  StatusResult getStatus() const             { return m_status; }

  /*
   * Get the id of the download.
   */
  const std::string&  getDid() const         { return m_did; }

  /*
   * Get the id of the "second" download.
   *
   * Set only if the "first" download is a metalink and is complete.
   */
  const std::string&  getFollowedBy() const  { return m_followedBy; }

  /*
   * Get the total length of the download.
   */
  uint64_t     getTotalLength() const        { return m_totalLength; }

  /*
   * Get the completed length of the download.
   */
  uint64_t     getCompletedLength() const    { return m_completedLength; }

  /*
   * Get the download speed of the download.
   */
  uint64_t     getDownloadSpeed() const      { return m_downloadSpeed; }

  /*
   * Get the verified length of the download.
   */
  uint64_t     getVerifiedLength() const     { return m_verifiedLength; }

  /*
   * Get the path (local file) of the download.
   */
  const std::string&  getPath() const        { return m_path; }

  /*
   * Get the download uris of the download.
   */
  const std::vector<std::string>&  getUris() const { return m_uris; }

 protected:
  std::shared_ptr<Aria2> mp_aria;
  StatusResult m_status;
  std::string m_did = "";
  std::string m_followedBy = "";
  uint64_t m_totalLength;
  uint64_t m_completedLength;
  uint64_t m_downloadSpeed;
  uint64_t m_verifiedLength;
  std::vector<std::string> m_uris;
  std::string m_path;
};

/**
 * A tool to download things.
 *
 * A Downloader manages `Download` using aria2 in the background.
 * `Downloader` is threadsafe.
 * However, the returned `Download`s are NOT threadsafe.
 */
class Downloader
{
 public: // types
  typedef std::vector<std::pair<std::string, std::string>> Options;

 public: // functions
  /*
  * Create a new Downloader object.
  *
  * @param sessionFileDir: The directory where aria2 will store its session file.
  */
  explicit Downloader(std::string sessionFileDir);
  virtual ~Downloader();

  void close();

  /**
   * Start a new download.
   *
   * This method is thread safe and returns a pointer to a newly created
   * `Download` or an existing one with a matching URI. In the latter case
   * the options parameter is ignored, which can lead to surprising results.
   * For example, if the old and new download requests (sharing the same URI)
   * have different values for the download directory or output file name
   * options, after the download is reported to be complete the downloaded file
   * will be present only at the location specified for the first request.
   *
   * User should call `update` on the returned `Download` to have an accurate status.
   *
   * @param uri: The uri of the thing to download.
   * @param downloadDir: The download directory where the thing should be stored (takes precedence over any "dir" in `options`).
   * @param options: A series of pair <option_name, option_value> to pass to aria.
   * @return: The newly created Download.
   */
  std::shared_ptr<Download> startDownload(const std::string& uri, const std::string& downloadDir, Options options = {});

  /**
   * Get a download corrsponding to a download id (did)
   * User should call `update` on the returned `Download` to have an accurate status.
   *
   * @param did: The download id to search for.
   * @return: The Download corresponding to did.
   * @throw: Throw std::out_of_range if did is not found.
   */
  std::shared_ptr<Download> getDownload(const std::string& did);

  /**
   * Get the number of downloads currently managed.
   */
  size_t getNbDownload() const;

  /**
   * Get the ids of the managed downloads.
   */
  std::vector<std::string> getDownloadIds() const;

 private: // data
  mutable std::mutex m_lock;
  std::map<std::string, std::shared_ptr<Download>> m_knownDownloads;
  std::shared_ptr<Aria2> mp_aria;
};
}

#endif
