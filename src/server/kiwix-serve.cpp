
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

#include <getopt.h>
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

void usage()
{
  std::cout << "Usage:" << std::endl
            << "\tkiwix-serve [OPTIONS] ZIM_PATH+" << std::endl
            << "\tkiwix-serve --library [OPTIONS] LIBRARY_PATH" << std::endl
            << std::endl

            << "Purpose:" << std::endl
            << "\tDeliver ZIM file(s) articles via HTTP"
            << std::endl << std::endl

            << "Mandatory arguments:" << std::endl
            << "\tLIBRARY_PATH\t\tXML library file path listing ZIM file to serve. To be used only with the --library argument."
            << std::endl
            << "\tZIM_PATH\t\tZIM file path(s)"
            << std::endl << std::endl

            << "Optional arguments:" << std::endl << std::endl
            << "\t-h, --help\t\tPrint this help" << std::endl << std::endl
            << "\t-a, --attachToProcess\tExit if given process id is not running anymore" << std::endl
            << "\t-d, --daemon\t\tDetach the HTTP server daemon from the main process" << std::endl
            << "\t-i, --address\t\tListen only on this ip address, all available ones otherwise" << std::endl
            << "\t-M, --monitorLibrary\tMonitor the XML library file and reload it automatically" << std::endl
            << "\t-m, --nolibrarybutton\tDon't print the builtin home button in the builtin top bar overlay" << std::endl
            << "\t-n, --nosearchbar\tDon't print the builtin bar overlay on the top of each served page" << std::endl
            << "\t-b, --blockexternal\tPrevent users from directly accessing external links" << std::endl
            << "\t-p, --port\t\tTCP port on which to listen to HTTP requests (default: 80)" << std::endl
            << "\t-r, --urlRootLocation\tURL prefix on which the content should be made available (default: /)" << std::endl
            << "\t-s, --searchLimit\tMaximun number of zim in a fulltext multizim search (default: No limit)" << std::endl
            << "\t-t, --threads\t\tNumber of threads to run in parallel (default: " << DEFAULT_THREADS << ")" << std::endl
            << "\t-v, --verbose\t\tPrint debug log to STDOUT" << std::endl
            << "\t-V, --version\t\tPrint software version" << std::endl
            << "\t-z, --nodatealiases\tCreate URL aliases for each content by removing the date" << std::endl
            << "\t-c, --customIndex\tAdd path to custom index.html for welcome page" << std::endl
            << "\t-L, --ipConnectionLimit\tMax number of (concurrent) connections per IP (default: infinite, recommended: >= 6)" << std::endl
            << std::endl

            << "Documentation:" << std::endl
            << "\tSource code\t\thttps://github.com/kiwix/kiwix-tools" << std::endl
            << "\tMore info\t\thttps://wiki.kiwix.org/wiki/Kiwix-serve" << std::endl
            << std::endl;
}

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

volatile sig_atomic_t waiting = false;
volatile sig_atomic_t libraryMustBeReloaded = false;

#ifndef _WIN32
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

int main(int argc, char** argv)
{
#ifndef _WIN32
  setup_sighandlers();
#endif

  std::string rootLocation = "/";
  auto library = std::make_shared<kiwix::Library>();
  unsigned int nb_threads = DEFAULT_THREADS;
  std::vector<std::string> zimPathes;
  std::string libraryPath;
  std::string rootPath;
  std::string address;
  std::string customIndexPath="";
  std::string indexTemplateString="";
  int serverPort = 80;
  int daemonFlag [[gnu::unused]] = false;
  int libraryFlag = false;
  bool noLibraryButtonFlag = false;
  bool noSearchBarFlag = false;
  bool noDateAliasesFlag = false;
  bool blockExternalLinks = false;
  bool isVerboseFlag = false;
  bool monitorLibrary = false;
  unsigned int PPID = 0;
  int ipConnectionLimit = 0;
  int searchLimit = 0;

  static struct option long_options[]
      = {{"daemon", no_argument, 0, 'd'},
         {"help", no_argument, 0, 'h'},
         {"verbose", no_argument, 0, 'v'},
         {"version", no_argument, 0, 'V'},
         {"library", no_argument, 0, 'l'},
         {"nolibrarybutton", no_argument, 0, 'm'},
         {"nodatealiases", no_argument, 0, 'z'},
         {"nosearchbar", no_argument, 0, 'n'},
         {"blockexternallinks", no_argument, 0, 'b'},
         {"attachToProcess", required_argument, 0, 'a'},
         {"port", required_argument, 0, 'p'},
         {"address", required_argument, 0, 'i'},
         {"threads", required_argument, 0, 't'},
         {"urlRootLocation", required_argument, 0, 'r'},
         {"customIndex", required_argument, 0, 'c'},
         {"monitorLibrary", no_argument, 0, 'M'},
         {"ipConnectionLimit", required_argument, 0, 'L'},
         {"searchLimit", required_argument, 0, 's'},
         {0, 0, 0, 0}};

  std::set<int> usedOptions;
  /* Argument parsing */
  while (true) {
    int option_index = 0;
    int c
        = getopt_long(argc, argv, "hzmnbdvVla:p:f:t:r:i:c:ML:s:", long_options, &option_index);

    if (c != -1) {
      auto insertRes = usedOptions.insert(c);
      if (!insertRes.second) {
        std::cerr << "Multiple values of same option are not allowed." << std::endl;
        exit(1);
      }
      switch (c) {
        case 'h':
          usage();
          return 0;
        case 'd':
          daemonFlag = true;
          break;
        case 'v':
          isVerboseFlag = true;
          break;
        case 'V':
          version();
          return 0;
        case 'l':
          libraryFlag = true;
          break;
        case 'n':
          noSearchBarFlag = true;
          break;
        case 'b':
          blockExternalLinks = true;
          break;
        case 'z':
          noDateAliasesFlag = true;
          break;
        case 'm':
          noLibraryButtonFlag = true;
          break;
        case 'p':
          serverPort = atoi(optarg);
          break;
        case 'a':
          PPID = atoi(optarg);
          break;
        case 'i':
          address = std::string(optarg);
          break;
        case 't':
          nb_threads = atoi(optarg);
          break;
        case 'r':
          rootLocation = std::string(optarg);
          break;
        case 'c':
          customIndexPath = std::string(optarg);
          break;
        case 'M':
          monitorLibrary = true;
          break;
        case 'L':
          ipConnectionLimit = atoi(optarg);
          break;
        case 's':
          searchLimit = atoi(optarg);
          break;
        case '?':
          usage();
          return 2;
      }
    } else {
      if (optind < argc) {
        if (libraryFlag) {
          libraryPath = argv[optind++];
        } else {
          while (optind < argc)
            zimPathes.push_back(std::string(argv[optind++]));
        }
      }
      break;
    }
  }

  /* Print usage)) if necessary */
  if (zimPathes.empty() && libraryPath.empty()) {
    usage();
    exit(1);
  }

  /* Setup the library manager and get the list of books */
  kiwix::Manager manager(library);
  std::vector<std::string> libraryPaths;
  if (libraryFlag) {
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
        std::cerr << "Unable to add the ZIM file '" << *it
             << "' to the internal library." << std::endl;
        exit(1);
      }
    }
  }
  auto libraryFileTimestamp = newestFileTimestamp(libraryPaths);
  auto curLibraryFileTimestamp = libraryFileTimestamp;

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
  kiwix::Server::Configuration configuration(library, nameMapper);

  if (!customIndexPath.empty()) {
    try {
      indexTemplateString = loadCustomTemplate(customIndexPath);
    } catch (std::runtime_error& e) {
      std::cerr << "ERROR: " << e.what() << std::endl;
      exit(1);
    }
  }

  configuration.setAddress(address);
  configuration.setRoot(rootLocation);
  configuration.setPort(serverPort);
  configuration.setNbThreads(nb_threads);
  configuration.setVerbose(isVerboseFlag);
  configuration.setTaskbar(!noSearchBarFlag, !noLibraryButtonFlag);
  configuration.setBlockExternalLinks(blockExternalLinks);
  configuration.setIndexTemplateString(indexTemplateString);
  configuration.setIpConnectionLimit(ipConnectionLimit);
  configuration.setMultiZimSearchLimit(searchLimit);

  kiwix::Server server(configuration);
  if (! server.start()) {
    exit(1);
  }

  std::string url = "http://" + server.getAddress() + ":" + std::to_string(server.getPort()) + normalizeRootUrl(rootLocation);
  std::cout << "The Kiwix server is running and can be accessed in the local network at: "
            << url << std::endl;

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
      libraryMustBeReloaded += curLibraryFileTimestamp > libraryFileTimestamp;
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
