
/*
 * Copyright 2009-2019 Emmanuel Engelhart <kelson@kiwix.org>
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

#include <docopt/docopt.h>
#include <kiwix/manager.h>
#include <kiwix/server.h>
#include <kiwix/name_mapper.h>
#include <kiwix/tools.h>

#ifdef _WIN32
# include <windows.h>
#else
# include <unistd.h>
# include <signal.h>
#endif
#include <sys/stat.h>

#ifdef __APPLE__
# import <sys/sysctl.h>
# import <sys/types.h>
# define MIBSIZE 4
#endif

#include "../version.h"

#define DEFAULT_THREADS 4
#define LITERAL_AS_STR(A) #A
#define AS_STR(A) LITERAL_AS_STR(A)


static const char USAGE[] =
R"(Deliver ZIM file(s) articles via HTTP

Usage:
 kiwix-serve [options] ZIMPATH ...
 kiwix-serve [options] (-l | --library) LIBRARYPATH
 kiwix-serve -h | --help
 kiwix-serve -V | --version

Mandatory arguments:
  LIBRARYPATH  XML library file path listing ZIM file to serve. To be used only with the --library argument."
  ZIMPATH      ZIM file path(s)

Options:
 -h --help                               Print this help
 -a <pid> --attachToProcess=<pid>        Exit if given process id is not running anymore [default: 0]
 --catalogOnly                           Serve only the library catalog
 --contentServerURL=<url>                Root URL of the server serving ZIM content for this library
 -d --daemon                             Detach the HTTP server daemon from the main process
 -i <address> --address=<address>        Listen only on the specified IP address. Specify 'ipv4', 'ipv6' or 'all' to listen on all IPv4, IPv6 or both types of addresses, respectively [default: all]
 -M --monitorLibrary                     Monitor the XML library file and reload it automatically
 -m --nolibrarybutton                    Don't print the builtin home button in the builtin top bar overlay
 -n --nosearchbar                        Don't print the builtin bar overlay on the top of each served page
 -b --blockexternal                      Prevent users from directly accessing external links
 -p <port> --port=<port>                 Port on which to listen to HTTP requests [default: 80]
 -r <root> --urlRootLocation=<root>      URL prefix on which the content should be made available [default: /]
 -s <limit> --searchLimit=<limit>        Maximun number of zim in a fulltext multizim search [default: 0]
 -t <threads> --threads=<threads>        Number of threads to run in parallel [default: )" AS_STR(DEFAULT_THREADS) R"(]
 -v --verbose                            Print debug log to STDOUT
 -V --version                            Print software version
 -z --nodatealiases                      Create URL aliases for each content by removing the date
 -c <path> --customIndex=<path>          Add path to custom index.html for welcome page
 -L <limit> --ipConnectionLimit=<limit>  Max number of (concurrent) connections per IP [default: 0] (recommended: >= 6)
 -k --skipInvalid                        Startup even when ZIM files are invalid (those will be skipped)

Documentation:
  Source code   https://github.com/kiwix/kiwix-tools
  More info     https://wiki.kiwix.org/wiki/Kiwix-serve
                https://kiwix-tools.readthedocs.io/en/latest/kiwix-serve.html
)";

std::string loadCustomTemplate (std::string customIndexPath) {
  customIndexPath = kiwix::isRelativePath(customIndexPath) ?
                      kiwix::computeAbsolutePath(kiwix::getCurrentDirectory(), customIndexPath) :
                      customIndexPath;
  if (!kiwix::fileReadable(customIndexPath)) {
    throw std::runtime_error("No such file exist (or file is not readable) " + customIndexPath);
  }
  if (kiwix::getMimeTypeForFile(customIndexPath) != "text/html") {
    throw std::runtime_error("Invalid File Mime Type " + kiwix::getMimeTypeForFile(customIndexPath));
  }
  std::string indexTemplateString = kiwix::getFileContent(customIndexPath);

  if (indexTemplateString.empty()) {
    throw std::runtime_error("Unreadable or empty file " + customIndexPath);
  }
  return indexTemplateString;
}

inline std::string normalizeRootUrl(std::string rootUrl)
{
  while ( !rootUrl.empty() && rootUrl.back() == '/' )
    rootUrl.pop_back();

  while ( !rootUrl.empty() && rootUrl.front() == '/' )
    rootUrl = rootUrl.substr(1);
  return rootUrl.empty() ? rootUrl : "/" + rootUrl;
}

#ifndef _WIN32
volatile sig_atomic_t waiting = false;
volatile sig_atomic_t libraryMustBeReloaded = false;
void handle_sigterm(int signum)
{
    if ( waiting == false ) {
        _exit(signum);
    }
    waiting = false;
}

void handle_sighup(int signum)
{
  libraryMustBeReloaded = true;
}

typedef void (*SignalHandler)(int);

void set_signal_handler(int sig, SignalHandler handler)
{
    struct sigaction sa;
    sigaction(sig, NULL, &sa);
    sa.sa_handler = handler;
    sigaction(sig, &sa, NULL);
}

void setup_sighandlers()
{
    set_signal_handler(SIGTERM, &handle_sigterm);
    set_signal_handler(SIGINT,  &handle_sigterm);
    set_signal_handler(SIGHUP,  &handle_sighup);
}
#else
bool waiting = false;
bool libraryMustBeReloaded = false;
#endif

uint64_t fileModificationTime(const std::string& path)
{
#if defined(_WIN32) && !defined(stat)
#define stat _stat
#endif
  struct stat fileStatData;
  if ( stat(path.c_str(), &fileStatData) == 0 ) {
    return fileStatData.st_mtime;
  }
  return 0;
#ifdef _WIN32
#undef stat
#endif
}

uint64_t newestFileTimestamp(const std::vector<std::string>& paths)
{
  uint64_t t = 0;
  for ( const auto& p : paths ) {
    t = std::max(t, fileModificationTime(p));
  }

  return t;
}

bool reloadLibrary(kiwix::Manager& mgr, const std::vector<std::string>& paths)
{
    try {
      std::cout << "Loading the library from the following files:\n";
      for ( const auto& p : paths ) {
        std::cout << "\t" << p << std::endl;
      }
      mgr.reload(paths);
      std::cout << "The library was successfully loaded." << std::endl;
      return true;
    } catch ( const std::runtime_error& err ) {
      std::cerr << "ERROR: " << err.what() << std::endl;
      std::cerr << "Errors encountered while loading the library." << std::endl;
      return false;
    }
}

// docopt::value::isLong() is counting repeated values.
// It doesn't check if the string can be parsed as long.
// (Contrarly to `asLong` which will try to convert string to long)
// See https://github.com/docopt/docopt.cpp/issues/62
// `isLong` is a small helper to get if the value can be parsed as long.
inline bool isLong(const docopt::value& v) {
  try {
    v.asLong();
    return true;
  } catch (...) {
    return false;
  }
}

#define FLAG(NAME, VAR) if (arg.first == NAME) { VAR = arg.second.asBool(); continue; }
#define STRING(NAME, VAR) if (arg.first == NAME && arg.second.isString() ) { VAR = arg.second.asString(); continue; }
#define STRING_LIST(NAME, VAR, ERRORSTR) if (arg.first == NAME) { if (arg.second.isStringList()) { VAR = arg.second.asStringList(); continue; } else { errorString = ERRORSTR; break; } }
#define INT(NAME, VAR, ERRORSTR) if (arg.first == NAME ) { if (isLong(arg.second)) { VAR = arg.second.asLong(); continue; } else { errorString = ERRORSTR; break; } }

// Older version of docopt doesn't declare Options. Let's declare it ourself.
using Options = std::map<std::string, docopt::value>;

int main(int argc, char** argv)
{
#ifndef _WIN32
  setup_sighandlers();
#endif

  std::string rootLocation = "/";
  auto library = kiwix::Library::create();
  unsigned int nb_threads = DEFAULT_THREADS;
  std::vector<std::string> zimPathes;
  std::string libraryPath;
  std::string rootPath;
  std::string address;
  std::string customIndexPath="";
  std::string indexTemplateString="";
  int serverPort = 80;
  bool catalogOnlyFlag = false;
  std::string contentServerURL;
  bool daemonFlag [[gnu::unused]] = false;
  bool helpFlag = false;
  bool noLibraryButtonFlag = false;
  bool noSearchBarFlag = false;
  bool noDateAliasesFlag = false;
  bool blockExternalLinks = false;
  bool isVerboseFlag = false;
  bool monitorLibrary = false;
  bool versionFlag = false;
  unsigned int PPID = 0;
  int ipConnectionLimit = 0;
  int searchLimit = 0;
  bool skipInvalid = false;

  std::string errorString;

  Options args;
  try {
    args = docopt::docopt_parse(USAGE, {argv+1, argv+argc}, false, false);
  } catch (docopt::DocoptArgumentError const & error) {
    std::cerr << error.what() << std::endl;
    std::cerr << USAGE << std::endl;
    return -1;
  }

  for (auto const& arg: args) {
    FLAG("--help", helpFlag)
    FLAG("--catalogOnly", catalogOnlyFlag)
    STRING("--contentServerURL", contentServerURL)
    FLAG("--daemon", daemonFlag)
    FLAG("--verbose", isVerboseFlag)
    FLAG("--nosearchbar", noSearchBarFlag)
    FLAG("--blockexternal", blockExternalLinks)
    FLAG("--nodatealiases", noDateAliasesFlag)
    FLAG("--nolibrarybutton",noLibraryButtonFlag)
    FLAG("--monitorLibrary", monitorLibrary)
    FLAG("--skipInvalid", skipInvalid)
    FLAG("--version", versionFlag)
    STRING("LIBRARYPATH", libraryPath)
    INT("--port", serverPort, "Port must be an integer")
    INT("--attachToProcess", PPID, "Process to attach must be an integer")
    STRING("--address", address)
    INT("--threads", nb_threads, "Number of threads must be an integer")
    STRING("--urlRootLocation", rootLocation)
    STRING("--customIndex", customIndexPath)
    INT("--ipConnectionLimit", ipConnectionLimit, "IP connection limit must be an integer")
    INT("--searchLimit", searchLimit, "Search limit must be an integer")
    STRING_LIST("ZIMPATH", zimPathes, "ZIMPATH must be a string list")
 }

 if (!errorString.empty()) {
   std::cerr << errorString << std::endl;
   std::cerr << USAGE << std::endl;
   return -1;
 }

 if (helpFlag) {
   std::cout << USAGE << std::endl;
   return 0;
 }

 if (versionFlag) {
   version();
   return 0;
 }

  /* Setup the library manager and get the list of books */
  kiwix::Manager manager(library);
  std::vector<std::string> libraryPaths;
  if (!libraryPath.empty()) {
    libraryPaths = kiwix::split(libraryPath, ";");
    if ( !reloadLibrary(manager, libraryPaths) ) {
      exit(1);
    }

    /* Check if the library is not empty (or only remote books)*/
    if (library->getBookCount(true, false) == 0) {
      std::cerr << "The XML library file '" << libraryPath
           << "' is empty (or has only remote books)." << std::endl;
    }
  } else {
    std::vector<std::string>::iterator it;
    for (it = zimPathes.begin(); it != zimPathes.end(); it++) {
      if (!manager.addBookFromPath(*it, *it, "", false)) {
        if (skipInvalid) {
          std::cerr << "Skipping invalid '" << *it << "' ...continuing" << std::endl;
        } else {
          std::cerr << "Unable to add the ZIM file '" << *it
               << "' to the internal library." << std::endl;
          exit(1);
        }
      }
    }
  }
  auto libraryFileTimestamp = newestFileTimestamp(libraryPaths);
  auto curLibraryFileTimestamp = libraryFileTimestamp;

  kiwix::IpMode ipMode = kiwix::IpMode::AUTO;

  if (address == "all") {
    address.clear();
    ipMode = kiwix::IpMode::ALL;
  } else if (address == "ipv4") {
    address.clear();
    ipMode = kiwix::IpMode::IPV4;
  } else if (address == "ipv6") {
    address.clear();
    ipMode = kiwix::IpMode::IPV6;
  }

#ifndef _WIN32
  /* Fork if necessary */
  if (daemonFlag) {
    pid_t pid;

    /* Fork off the parent process */
    pid = fork();
    if (pid < 0) {
      exit(1);
    }

    /* If we got a good PID, then
       we can exit the parent process. */
    if (pid > 0) {
      exit(0);
    }
  }
#endif

  auto nameMapper = std::make_shared<kiwix::UpdatableNameMapper>(library, noDateAliasesFlag);
  kiwix::Server server(library, nameMapper);

  if (!customIndexPath.empty()) {
    try {
      indexTemplateString = loadCustomTemplate(customIndexPath);
    } catch (std::runtime_error& e) {
      std::cerr << "ERROR: " << e.what() << std::endl;
      exit(1);
    }
  }

  server.setAddress(address);
  server.setRoot(rootLocation);
  server.setPort(serverPort);
  server.setNbThreads(nb_threads);
  server.setVerbose(isVerboseFlag);
  server.setTaskbar(!noSearchBarFlag, !noLibraryButtonFlag);
  server.setBlockExternalLinks(blockExternalLinks);
  server.setIndexTemplateString(indexTemplateString);
  server.setIpConnectionLimit(ipConnectionLimit);
  server.setMultiZimSearchLimit(searchLimit);
  server.setIpMode(ipMode);
  server.setCatalogOnlyMode(catalogOnlyFlag);
  while ( !contentServerURL.empty() && contentServerURL.back() == '/' )
    contentServerURL.pop_back();
  server.setContentServerUrl(contentServerURL);

  if (! server.start()) {
    exit(1);
  }

  std::string prefix = "http://";
  kiwix::IpAddress addresses = server.getAddress();
  std::string suffix = ":" + std::to_string(server.getPort()) + normalizeRootUrl(rootLocation);
  std::cout << "The Kiwix server is running and can be accessed in the local network at: " << std::endl;
  if(!addresses.addr.empty()) std::cout << "  - " << prefix << addresses.addr << suffix << std::endl;
  if(!addresses.addr6.empty()) std::cout << "  - " << prefix << "[" << addresses.addr6 << "]" <<  suffix << std::endl;

  /* Run endless (until PPID dies) */
  waiting = true;
  do {
    if (PPID > 0) {
#ifdef _WIN32
      HANDLE process = OpenProcess(SYNCHRONIZE, FALSE, PPID);
      DWORD ret = WaitForSingleObject(process, 0);
      CloseHandle(process);
      if (ret == WAIT_TIMEOUT) {
#elif __APPLE__
      int mib[MIBSIZE];
      struct kinfo_proc kp;
      size_t len = sizeof(kp);

      mib[0] = CTL_KERN;
      mib[1] = KERN_PROC;
      mib[2] = KERN_PROC_PID;
      mib[3] = PPID;

      int ret = sysctl(mib, MIBSIZE, &kp, &len, NULL, 0);
      if (ret != -1 && len > 0) {
#else /* Linux & co */
      std::string procPath = "/proc/" + std::to_string(PPID);
      if (access(procPath.c_str(), F_OK) != -1) {
#endif
      } else {
        waiting = false;
      }
    }

    kiwix::sleep(1000);

    if ( monitorLibrary ) {
      curLibraryFileTimestamp = newestFileTimestamp(libraryPaths);
      if ( !libraryMustBeReloaded ) {
        libraryMustBeReloaded = curLibraryFileTimestamp > libraryFileTimestamp;
      }
    }

    if ( libraryMustBeReloaded && !libraryPaths.empty() ) {
      libraryFileTimestamp = curLibraryFileTimestamp;
      reloadLibrary(manager, libraryPaths);
      nameMapper->update();
      libraryMustBeReloaded = false;
    }
  } while (waiting);

  /* Stop the daemon */
  server.stop();
}
