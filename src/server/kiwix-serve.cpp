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
#include <kiwix/tools/otherTools.h>

#ifndef _WIN32
# include <unistd.h>
#endif

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
            << "\tDeliver ZIM file articles via HTTP"
            << std::endl << std::endl

            << "Mandatory arguments:" << std::endl
            << "\tLIBRARY_PATH\t\tis the XML library file path listing ZIM file to serve. To be used only with the --library argument."
            << std::endl
            << "\tZIM_PATH\t\tis the path of a ZIM file."
            << std::endl << std::endl

            << "Optional arguments:" << std::endl
            << "\t-a, --attachToProcess\texit if given process id is not running anymore" << std::endl
            << "\t-d, --daemon\t\tdetach the HTTP server daemon from the main process" << std::endl
            << "\t-i, --address\t\tlisten only on this ip address, all available ones otherwise" << std::endl
            << "\t-m, --nolibrarybutton\tdo not print the builtin home button in the builtin top bar overlay" << std::endl
            << "\t-n, --nosearchbar\tdo not print the builtin bar overlay on the top of each served page" << std::endl
            << "\t-p, --port\t\tTCP port on which to listen to HTTP requests (default: 80)" << std::endl
            << "\t-r, --urlRootLocation\tURL prefix on which the content should be made available (default: /)" << std::endl
            << "\t-t, --threads\t\tnumber of threads to run in parallel (default: " << DEFAULT_THREADS << ")" << std::endl
            << "\t-v, --verbose\t\tprint debug log to STDOUT" << std::endl
            << "\t-V, --version\t\tprint software version" << std::endl
            << "\t-z, --nodatealiases\tcreate URL aliases for each content by removing the date" << std::endl
            << std::endl

            << "Documentation:" << std::endl
            << "\tSource code\t\thttps://github.com/kiwix/kiwix-tools" << std::endl
            << "\tMore info\t\thttps://wiki.kiwix.org/wiki/Kiwix-serve" << std::endl
            << std::endl;
}

int main(int argc, char** argv)
{
  std::string rootLocation = "";
  kiwix::Library library;
  unsigned int nb_threads = DEFAULT_THREADS;
  vector<string> zimPathes;
  string libraryPath;
  string rootPath;
  string address;
  int serverPort = 80;
  int daemonFlag [[gnu::unused]] = false;
  int libraryFlag = false;
  bool noLibraryButtonFlag = false;
  bool noSearchBarFlag = false;
  bool noDateAliasesFlag = false;
  bool isVerboseFlag = false;
  string PPIDString;
  unsigned int PPID = 0;

  static struct option long_options[]
      = {{"daemon", no_argument, 0, 'd'},
         {"verbose", no_argument, 0, 'v'},
         {"version", no_argument, 0, 'V'},
         {"library", no_argument, 0, 'l'},
         {"nolibrarybutton", no_argument, 0, 'm'},
         {"nodatealiases", no_argument, 0, 'z'},
         {"nosearchbar", no_argument, 0, 'n'},
         {"attachToProcess", required_argument, 0, 'a'},
         {"port", required_argument, 0, 'p'},
         {"address", required_argument, 0, 'i'},
         {"threads", required_argument, 0, 't'},
         {"urlRootLocation", required_argument, 0, 'r'},
         {0, 0, 0, 0}};

  /* Argument parsing */
  while (true) {
    int option_index = 0;
    int c
        = getopt_long(argc, argv, "mndvVla:p:f:t:r:", long_options, &option_index);

    if (c != -1) {
      switch (c) {
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
          PPIDString = string(optarg);
          PPID = atoi(optarg);
          break;
        case 'i':
          address = string(optarg);
          break;
        case 't':
          nb_threads = atoi(optarg);
          break;
        case 'r':
          rootLocation = string(optarg);
          break;
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
  kiwix::Manager manager(&library);
  if (libraryFlag) {
    vector<string> libraryPaths = kiwix::split(libraryPath, ";");
    vector<string>::iterator itr;

    for (itr = libraryPaths.begin(); itr != libraryPaths.end(); ++itr) {
      if (!itr->empty()) {
        bool retVal = false;

        try {
          string libraryPath
              = isRelativePath(*itr)
                    ? computeAbsolutePath(getCurrentDirectory(), *itr)
                    : *itr;
          retVal = manager.readFile(libraryPath, true);
        } catch (...) {
          retVal = false;
        }

        if (!retVal) {
          cerr << "Unable to open the XML library file '" << *itr << "'."
               << endl;
          exit(1);
        }
      }
    }

    /* Check if the library is not empty (or only remote books)*/
    if (library.getBookCount(true, false) == 0) {
      cerr << "The XML library file '" << libraryPath
           << "' is empty (or has only remote books)." << endl;
    }
  } else {
    std::vector<std::string>::iterator it;
    for (it = zimPathes.begin(); it != zimPathes.end(); it++) {
      if (!manager.addBookFromPath(*it, *it, "", false)) {
        cerr << "Unable to add the ZIM file '" << *it
             << "' to the internal library." << endl;
        exit(1);
      }
    }
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

  kiwix::HumanReadableNameMapper nameMapper(library, noDateAliasesFlag);
  kiwix::Server server(&library, &nameMapper);
  server.setAddress(address);
  server.setRoot(rootLocation);
  server.setPort(serverPort);
  server.setNbThreads(nb_threads);
  server.setVerbose(isVerboseFlag);
  server.setTaskbar(!noSearchBarFlag, !noLibraryButtonFlag);

  if (! server.start()) {
    cerr << "Unable to instantiate the HTTP daemon. The port " << serverPort
         << " is maybe already occupied or need more permissions to be open. "
            "Please try as root or with a port number higher or equal to 1024."
         << endl;
    exit(1);
  }

  /* Run endless (until PPID dies) */
  bool waiting = true;
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
      string procPath = "/proc/" + string(PPIDString);
      if (access(procPath.c_str(), F_OK) != -1) {
#endif
      } else {
        waiting = false;
      }
    }

    kiwix::sleep(1000);
  } while (waiting);

  /* Stop the daemon */
  server.stop();
}
