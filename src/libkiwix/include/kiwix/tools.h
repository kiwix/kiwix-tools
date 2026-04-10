/*
 * Copyright 2021 Matthieu Gautier <mgautier@kymeria.fr>
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

#ifndef KIWIX_TOOLS_H
#define KIWIX_TOOLS_H

#include <string>
#include <vector>
#include <map>
#include <cstdint>
#include "common.h"

namespace kiwix
{

/**
 * An IPv4 and/or IPv6 address.
 */
struct IpAddress
{
    std::string addr;  /**< IPv4 address */
    std::string addr6; /**< IPv6 address */
};

typedef std::pair<std::string, std::string> LangNameCodePair;
typedef std::vector<LangNameCodePair> FeedLanguages;
typedef std::vector<std::string> FeedCategories;

/**
 * Return the current directory.
 *
 * @return the current directory (utf8 encoded)
 */
std::string getCurrentDirectory();

/** Return the path of the executable
 *
 * Some application may be packaged in auto extractible archive (Appimage) and the
 * real executable is different of the path of the archive.
 * If `realPathOnly` is true, return the path of the real executable instead of the
 * archive launched by the user.
 *
 * @param realPathOnly If we must return the real path of the executable.
 * @return the path of the executable (utf8 encoded)
 */
std::string getExecutablePath(bool realPathOnly = false);

/** Tell if the path is a relative path.
 *
 * This function is provided as a small helper. It is probably better to use native tools
 * to manipulate paths.
 *
 * @param path A utf8 encoded path.
 * @return true if the path is relative.
 */
bool isRelativePath(const std::string& path);

/** Append a path to another one.
 *
 * This function is provided as a small helper. It is probably better to use native tools
 * to manipulate paths.
 *
 * @param basePath the base path.
 * @param relativePath a path to add to the base path, must be a relative path.
 * @return The concatenation of the paths, using the right separator.
 */
std::string appendToDirectory(const std::string& basePath, const std::string& relativePath);

/** Remove the last element of a path.
 *
 * This function is provided as a small helper. It is probably better to use native tools
 * to manipulate paths.
 *
 * @param path a path.
 * @return The parent directory (or empty string if none).
 */
std::string removeLastPathElement(const std::string& path);

/** Get the last element of a path.
 *
 * This function is provided as a small helper. It is probably better to use native tools
 * to manipulate paths.
 *
 * @param path a path.
 * @return The base name of the path or empty string if none (ending with a separator).
 */
std::string getLastPathElement(const std::string& path);

/** Compute the absolute path of a relative path based on another one
 *
 * Equivalent to appendToDirectory followed by a normalization of the path.
 *
 * This function is provided as a small helper. It is probably better to use native tools
 * to manipulate paths.
 *
 * @param path the base path (if empty, current directory is taken).
 * @param relativePath the relative path.
 * @return a absolute path.
 */
std::string computeAbsolutePath(const std::string& path, const std::string& relativePath);

/** Compute the relative path of a path relative to another one
 *
 * This function is provided as a small helper. It is probably better to use native tools
 * to manipulate paths.
 *
 * @param path the base path.
 * @param absolutePath the absolute path to find the relative path for.
 * @return a relative path (pointing to absolutePath, relative to path).
 */
std::string computeRelativePath(const std::string& path, const std::string& absolutePath);

/** Sleep the current thread.
 *
 * This function is provided as a small helper. It is probably better to use native tools.
 *
 * @param milliseconds The number of milliseconds to wait for.
 */
void sleep(unsigned int milliseconds);

/** Split a string
 *
 * This function is provided as a small helper. It is probably better to use native tools.
 *
 * Assuming text = "foo:;bar;baz,oups;"
 *
 * split(text, ":;", true, true) => ["foo", ":", ";", "bar", ";", "baz,oups", ";"]
 * split(text, ":;", true, false) => ["foo", "bar", "baz,oups"] (default)
 * split(text, ":;", false, true) => ["foo", ":", "", ";", "bar", ";", "baz,oups", ";", ""]
 * split(text, ":;", false, false) => ["foo", "", "bar", "baz,oups", ""]
 *
 * @param str The string to split.
 * @param delims A string of potential delimiters.
 *               Each charater in the string can be a individual delimiters.
 * @param dropEmpty true if empty part must be dropped from the result.
 * @param keepDelim true if delimiter must be included from the result.
 * @return a list of part (potentially containing delimiters)
 */
std::vector<std::string> split(const std::string& str, const std::string& delims, bool dropEmpty=true, bool keepDelim = false);

/** Convert language code from iso2 code to iso3
 *
 * This function is provided as a small helper. It is probably better to use native tools
 * to manipulate locales.
 *
 * @param a2code a iso2 code string.
 * @return the corresponding iso3 code.
 * @throw std::out_of_range if iso2 code is not known.
 */
std::string converta2toa3(const std::string& a2code);

/** Extracts content from given file.
 *
 * This function provides content of a file provided it's path.
 *
 * @param path The absolute path provided in string format.
 * @return Content of corresponding file in string format.
 */
std::string getFileContent(const std::string& path);

/** Checks if file exists.
 *
 * This function returns boolean stating if file exists.
 *
 * @param path The absolute path provided in string format.
 * @return Boolean representing if file exists or not.
 */
bool fileExists(const std::string& path);

/** Checks if file is readable.
 *
 * This function returns boolean stating if file is readable.
 *
 * @param path The absolute path provided in string format.
 * @return Boolean representing if file is readale or not.
 */
bool fileReadable(const std::string& path);

/** Provides mimetype from filename.
 *
 * This function provides mimetype from file-name.
 *
 * @param filename string containing filename.
 * @return mimetype from filename in string format.
 */
std::string getMimeTypeForFile(const std::string& filename);

/** Provides all available network interfaces
 *
 * This function provides the available IPv4 and IPv6 network interfaces
 * as a map from the interface name to its IPv4 and/or IPv6 address(es).
 */
std::map<std::string, IpAddress> getNetworkInterfacesIPv4Or6();

/** Provides all available IPv4 network interfaces
 *
 * This function provides the available IPv4 network interfaces
 * as a map from the interface name to its IPv4 address.
 *
 * Provided for backward compatibility with libkiwix v13.1.0.
 */
std::map<std::string, std::string> getNetworkInterfaces();

/** Provides the best IP address
 * This function provides the best IP addresses for both ipv4 and ipv6 protocols,
 * in an IpAddress struct, based on the list given by getNetworkInterfacesIPv4Or6()
 */
IpAddress getBestPublicIps();

/** Provides the best IPv4 adddress
 * Equivalent to getBestPublicIp(false). Provided for backward compatibility
 * with libkiwix v13.1.0.
 */
std::string getBestPublicIp();

/** Converts file size to human readable format.
 *
 * This function will convert a number to its equivalent size using units.
 *
 * @param number file size in bytes.
 * @return a human-readable string representation of the size, e.g., "2.3 KB", "1.8 MB", "5.2 GB".
 */
std::string beautifyFileSize(uint64_t number);

/**
 * Load languages stored in an OPDS stream.
 *
 * @param content the OPDS stream.
 * @return vector containing pairs of language code and their corresponding full language name.
 */
FeedLanguages readLanguagesFromFeed(const std::string& content);

/**
 * Load categories stored in an OPDS stream .
 *
 * @param content the OPDS stream.
 * @return vector containing category strings.
 */
FeedCategories readCategoriesFromFeed(const std::string& content);

/**
 * Retrieve the full language name associated with a given ISO 639-3 language code.
 *
 * @param lang ISO 639-3 language code.
 * @return full language name.
 */
std::string getLanguageSelfName(const std::string& lang);

/**
 * Slugifies the filename by converting any characters reserved by the operating
 * system to '_'. Note filename is only the file name and not a path.
 *
 * @param filename Valid UTF-8 encoded file name string.
 * @return slugified string.
 */
std::string getSlugifiedFileName(const std::string& filename);
}
#endif // KIWIX_TOOLS_H
