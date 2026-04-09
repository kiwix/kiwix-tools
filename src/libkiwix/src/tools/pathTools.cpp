/*
 * Copyright 2011-2014 Emmanuel Engelhart <kelson@kiwix.org>
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

// Implement method defined in <kiwix/tools.h> and "tools/pathTools.h"
#include "tools.h"
#include "tools/pathTools.h"

#include <stdexcept>

#ifdef __APPLE__
#include <limits.h>
#include <mach-o/dyld.h>
#elif _WIN32
#include <direct.h>
#include <windows.h>
#include "shlwapi.h"
#define getcwd _getcwd  // stupid MSFT "deprecation" warning
#endif

#include "tools/stringTools.h"

#include <string.h>
#include <map>
#include <vector>
#include <sys/stat.h>
#include <sstream>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <algorithm>

#ifdef _WIN32
# define SEPARATOR "\\"
# include <io.h>
#else
# define SEPARATOR "/"
# include <unistd.h>
# include <sys/stat.h>
#endif

#include <fcntl.h>

#include <stdlib.h>

#ifndef PATH_MAX
#define PATH_MAX 1024
#endif

#ifdef _WIN32
std::string WideToUtf8(const std::wstring& wstr)
{
  auto needed_size = WideCharToMultiByte(CP_UTF8, 0, wstr.data(), wstr.size(), NULL, 0, NULL, NULL);
  std::string ret(needed_size, 0);
  WideCharToMultiByte(CP_UTF8, 0, wstr.data(), wstr.size(), &ret[0], needed_size, NULL, NULL);
  return ret;
}

std::wstring Utf8ToWide(const std::string& str)
{
  auto needed_size = MultiByteToWideChar(CP_UTF8, 0, str.data(), str.size(), NULL, 0);
  std::wstring ret(needed_size, 0);
  MultiByteToWideChar(CP_UTF8, 0, str.data(), str.size(), &ret[0], needed_size);
  return ret;
}
#endif

bool kiwix::isRelativePath(const std::string& path)
{
#ifdef _WIN32
  if (path.size() < 3 ) {
    return true;
  }
  if (path.substr(1, 2) == ":\\" || path.substr(0, 2) == "\\\\") {
    return false;
  }
  return true;
#else
  return path.empty() || path.substr(0, 1) == "/" ? false : true;
#endif
}

std::vector<std::string> normalizeParts(std::vector<std::string>& parts, bool absolute)
{
  std::vector<std::string> ret;
#ifdef _WIN32
  //Special case if we have a drive directory not at first.
  //Starts from there.
  auto it = find_if(parts.rbegin(), parts.rend(),
                    [](const std::string& p) ->bool
                      { return ((p.length() == 2 && p[1] == ':')
                             || (p.length() > 2 && p[0] == '\\' && p[1] == '\\')); });
  if (it != parts.rend()) {
    parts.erase(parts.begin(), it.base()-1);
  }
  // Special case for samba mount point starting with two "\\" ("\\\\samba\\foo")
  if (parts.size() > 2  && parts[0].empty() && parts[1].empty()) {
    parts.erase(parts.begin(), parts.begin()+2);
    parts[0] = "\\\\" + parts[0];
  }
  // Special case if we have a samba drive not at first.
  // Path is "..\\\\sambdadrive\\..\\.." So we will have an empty part.
  auto previous_empty = false;
  for (it = parts.rbegin(); it!=parts.rend(); it++) {
    if(it->empty()) {
      if (previous_empty) {
        it++;
        break;
      } else {
        previous_empty = true;
      }
    } else {
      previous_empty = false;
    }
  }
  if (it != parts.rend()) {
    parts.erase(parts.begin(), it.base()-1);
    parts[0] = "\\\\" + parts[0];
  }
#endif

  size_t index = 0;
  for (auto& part: parts) {
    index++;
    if (part == "..") {
      if (absolute) {
        // We try to remove as far as possible.
        if (ret.size() > 1) {
          ret.pop_back();
        }
      } else {
        // We remove only if we can remove it.
        // Else we add it.
        if (!ret.empty() && ret.back() != "..") {
          ret.pop_back();
        } else {
          ret.push_back("..");
        }
      }
      continue;
    }
    if (part == "") {
#ifndef _WIN32
      if (ret.empty() && (absolute || index<parts.size())) {
        ret.push_back("");
      }
#endif
      continue;
    }
    if (part == ".") {
      continue;
    }
    ret.push_back(part);
  }
#ifndef _WIN32
  if (absolute && ret.size() == 1 && ret.back() == "") {
    ret.push_back("");
  }
#endif
  return ret;
}

std::string kiwix::computeRelativePath(const std::string& path, const std::string& absolutePath)
{
  auto parts = kiwix::split(path, SEPARATOR, false);
  auto pathParts = normalizeParts(parts, false);
  parts = kiwix::split(absolutePath, SEPARATOR, false);
  auto absolutePathParts = normalizeParts(parts, true);

  unsigned int commonCount = 0;
  while (commonCount < pathParts.size()
         && commonCount < absolutePathParts.size()
         && pathParts[commonCount] == absolutePathParts[commonCount]) {
      commonCount++;
  }

  std::vector<std::string> relativeParts;
  for (unsigned int i = commonCount; i < pathParts.size(); i++) {
    relativeParts.push_back("..");
  }
  for (unsigned int i = commonCount; i < absolutePathParts.size(); i++) {
    relativeParts.push_back(absolutePathParts[i]);
  }
  auto ret = kiwix::join(normalizeParts(relativeParts, false), SEPARATOR);
  return ret;
}

std::string kiwix::computeAbsolutePath(const std::string& path, const std::string& relativePath)
{
  std::string absolutePath = path;
  if (path.empty()) {
    absolutePath = kiwix::getCurrentDirectory();
  }

  auto parts = kiwix::split(absolutePath, SEPARATOR, false);
  auto absoluteParts = normalizeParts(parts, true);
  parts = kiwix::split(relativePath, SEPARATOR, false);
  auto relativeParts = normalizeParts(parts, false);

  absoluteParts.insert(absoluteParts.end(), relativeParts.begin(), relativeParts.end());
  auto ret = kiwix::join(normalizeParts(absoluteParts, true), SEPARATOR);
  return ret;
}

std::string kiwix::removeLastPathElement(const std::string& path)
{
  auto parts_ = kiwix::split(path, SEPARATOR, false);
  auto parts = normalizeParts(parts_, false);
  if (!parts.empty()) {
    parts.pop_back();
  }
  auto ret = kiwix::join(parts, SEPARATOR);
  return ret;
}

std::string kiwix::appendToDirectory(const std::string& directoryPath, const std::string& filename)
{
  std::string newPath = directoryPath;
  if (!directoryPath.empty() && directoryPath.back() != SEPARATOR[0]) {
    newPath += SEPARATOR;
  }
  newPath += filename;
  return newPath;
}

std::string kiwix::getLastPathElement(const std::string& path)
{
  auto parts_ = kiwix::split(path, SEPARATOR);
  auto parts = normalizeParts(parts_, false);
  if (parts.empty()) {
    return "";
  }
  auto ret = parts.back();
  return ret;
}

unsigned int getFileSize(const std::string& path)
{
#ifdef _WIN32
  struct _stat filestatus;
  _stat(path.c_str(), &filestatus);
#else
  struct stat filestatus;
  stat(path.c_str(), &filestatus);
#endif

  return filestatus.st_size / 1024;
}

std::string getFileSizeAsString(const std::string& path)
{
  std::ostringstream convert;
  convert << getFileSize(path);
  return convert.str();
}

std::string kiwix::getFileContent(const std::string& path)
{
#ifdef _WIN32
  auto wpath = Utf8ToWide(path);
  auto fd = _wopen(wpath.c_str(), _O_RDONLY | _O_BINARY);
#else
  auto fd = open(path.c_str(), O_RDONLY);
#endif
  std::string content;
  if (fd != -1) {
#ifdef _WIN32
    auto size = _lseeki64(fd, 0, SEEK_END);
#else
    auto size = lseek(fd, 0, SEEK_END);
#endif
    content.resize(size);
#ifdef _WIN32
    _lseeki64(fd, 0, SEEK_SET);
#else
    lseek(fd, 0, SEEK_SET);
#endif
    auto p = (char*)content.data();
    while (size) {
      auto readsize = size > 2048 ? 2048 : size;
      readsize = ::read(fd, p, readsize);
      p += readsize;
      size -= readsize;
    }
    close(fd);
  }
  return content;
}

bool kiwix::fileExists(const std::string& path)
{
#ifdef _WIN32
  return (_waccess_s(Utf8ToWide(path).c_str(), 0) == 0);
#else
  return (access(path.c_str(), F_OK) == 0);
#endif
}

bool kiwix::fileReadable(const std::string& path)
{
#ifdef _WIN32
  return (_waccess_s(Utf8ToWide(path).c_str(), 4) == 0);
#else
  return (access(path.c_str(), R_OK) == 0);
#endif
}

std::string makeTmpDirectory()
{
#ifdef _WIN32
  wchar_t cbase[MAX_PATH];
  wchar_t ctmp[MAX_PATH];
  GetTempPathW(MAX_PATH-14, cbase);
  // This create a file for us, ensure it is unique.
  // So we need to delete it and create the directory using the same name.
  GetTempFileNameW(cbase, L"kiwix", 0, ctmp);
  DeleteFileW(ctmp);
  _wmkdir(ctmp);
  return WideToUtf8(ctmp);
#else
  char _template_array[] = {"/tmp/libkiwix_XXXXXX"};
  std::string dir = mkdtemp(_template_array);
  return dir;
#endif
}

/* Try to create a link and if does not work then make a copy */
bool copyFile(const std::string& sourcePath, const std::string& destPath)
{
#ifdef _WIN32
  return CopyFileW(Utf8ToWide(sourcePath).c_str(), Utf8ToWide(destPath).c_str(), 1);
#else
  try {
    if (link(sourcePath.c_str(), destPath.c_str()) != 0) {
      std::ifstream infile(sourcePath.c_str(), std::ios_base::binary);
      std::ofstream outfile(destPath.c_str(), std::ios_base::binary);
      outfile << infile.rdbuf();
    }
  } catch (std::exception& e) {
    std::cerr << e.what() << std::endl;
    return false;
  }

  return true;
#endif
}

std::string kiwix::getExecutablePath(bool realPathOnly)
{
  if (!realPathOnly) {
    char* cAppImage = ::getenv("APPIMAGE");
    if (cAppImage) {
      char* cArgv0 = ::getenv("ARGV0");
      char* cOwd = ::getenv("OWD");
      if (cArgv0 && cOwd) {
        auto ret = appendToDirectory(cOwd, cArgv0);
        return ret;
      }
    }
  }

#ifdef _WIN32
  std::wstring binRootPath(PATH_MAX, 0);
  GetModuleFileNameW(NULL, &binRootPath[0], PATH_MAX);
  std::string ret = WideToUtf8(binRootPath);
  return ret;
#elif __APPLE__
  char binRootPath[PATH_MAX];
  uint32_t max = (uint32_t)PATH_MAX;
  _NSGetExecutablePath(binRootPath, &max);
  return std::string(binRootPath);
#else
  char binRootPath[PATH_MAX];
  ssize_t size = readlink("/proc/self/exe", binRootPath, PATH_MAX);
  if (size != -1) {
    return std::string(binRootPath, size);
  }
  return "";
#endif
}

bool writeTextFile(const std::string& path, const std::string& content)
{
#ifdef _WIN32
  auto wpath = Utf8ToWide(path);
  auto fd = _wopen(wpath.c_str(), _O_WRONLY | _O_CREAT | _O_TRUNC, S_IWRITE);
#else
  auto fd = open(path.c_str(), O_WRONLY | O_CREAT | O_TRUNC,
                               S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
#endif
  if (fd == -1)
    return false;

  if (write(fd, content.c_str(), content.size()) != (long)content.size()) {
    close(fd);
    return false;
  }
  close(fd);
  return true;
}

std::string kiwix::getCurrentDirectory()
{
#ifdef _WIN32
  wchar_t* a_cwd = _wgetcwd(NULL, 0);
  std::string ret = WideToUtf8(a_cwd);
  free(a_cwd);
#else
  char* a_cwd = getcwd(NULL, 0);
  std::string ret(a_cwd);
  free(a_cwd);
#endif
  return ret;
}

static std::map<std::string, std::string> extMimeTypes = {
 { "html", "text/html"},
 { "htm", "text/html"},
 { "png", "image/png"},
 { "tiff", "image/tiff"},
 { "tif", "image/tiff"},
 { "jpeg", "image/jpeg"},
 { "jpg", "image/jpeg"},
 { "gif", "image/gif"},
 { "ico", "image/x-icon"},
 { "svg", "image/svg+xml"},
 { "txt", "text/plain"},
 { "xml", "text/xml"},
 { "pdf", "application/pdf"},
 { "ogg", "application/ogg"},
 { "js", "application/javascript"},
 { "json", "application/json"},
 { "css", "text/css"},
 { "otf", "application/vnd.ms-opentype"},
 { "ttf", "application/font-ttf"},
 { "woff", "application/font-woff"},
 { "vtt", "text/vtt"}
};

/* Try to get the mimeType from the file extension */
std::string kiwix::getMimeTypeForFile(const std::string& filename)
{
  std::string mimeType = "text/plain";
  auto pos = filename.find_last_of(".");

  if (pos != std::string::npos) {
    std::string extension = filename.substr(pos + 1);

    auto it = extMimeTypes.find(extension);
    if (it != extMimeTypes.end()) {
      mimeType = it->second;
    } else {
      it = extMimeTypes.find(kiwix::lcAll(extension));
      if (it != extMimeTypes.end()) {
        mimeType = it->second;
      }
    }
  }

  return mimeType;
}
