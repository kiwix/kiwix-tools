#ifndef KIWIX_CLI_VERSION_H_
#define KIWIX_CLI_VERSION_H_

#ifndef KIWIX_TOOLS_VERSION
  #define KIWIX_TOOLS_VERSION "3.8.2"
#endif

#include <kiwix/version.h>
#include <zim/version.h>
#include <iostream>

inline void print_version()
{
  std::cout << "kiwix-tools " << KIWIX_TOOLS_VERSION << std::endl << std::endl;
  kiwix::printVersions();
  std::cout << std::endl;
  zim::printVersions();
}

#endif
