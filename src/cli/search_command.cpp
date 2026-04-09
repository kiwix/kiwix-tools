#include <docopt/docopt.h>
#include <zim/search.h>
#include <zim/suggestion.h>
#include <kiwix/spelling_correction.h>
#include <xapian.h>
#include <iostream>
#include <filesystem>

#ifdef _WIN32
#include <windows.h>
#include <shlobj.h>
#else
#include <cstdlib>
#include <unistd.h>
#endif

#include "version.h"

using Options = std::map<std::string, docopt::value>;

static const char USAGE[] =
R"(Search articles in a ZIM file

Usage:
  kiwix search [options] ZIM PATTERN
  kiwix search -h | --help

Arguments:
  ZIM       The full path of the ZIM file
  PATTERN   Word(s) to search for

Options:
  -s --suggestion    Suggest article titles based on PATTERN prefix
  --spelling         Suggest corrected spelling for PATTERN
  -v --verbose       Give details about the search process
  -h --help          Print this help
)";

static std::filesystem::path getCacheDir()
{
#ifdef _WIN32
  char path[MAX_PATH];
  if (SUCCEEDED(SHGetFolderPathA(NULL, CSIDL_LOCAL_APPDATA, NULL, 0, path))) {
    return std::filesystem::path(path) / "kiwix" / "cache";
  }
  const char* localAppData = std::getenv("LOCALAPPDATA");
  if (localAppData) {
    return std::filesystem::path(localAppData) / "kiwix" / "cache";
  }
#else
  const char* home = std::getenv("HOME");
  if (home) {
    return std::filesystem::path(home) / ".cache" / "kiwix";
  }
#endif
  return std::filesystem::current_path() / ".kiwix-cache";
}

int run_search_command(int argc, char** argv)
{
  Options args;
  try {
    args = docopt::docopt_parse(USAGE, {argv + 1, argv + argc}, false, false);
  } catch (docopt::DocoptArgumentError const& error) {
    std::cerr << error.what() << std::endl;
    std::cerr << USAGE << std::endl;
    return -1;
  }

  if (args.at("--help").asBool()) {
    std::cout << USAGE << std::endl;
    return 0;
  }

  auto zimPath = args.at("ZIM").asString();
  auto pattern = args.at("PATTERN").asString();
  auto verbose = args.at("--verbose").asBool();

  try {
    zim::Archive archive(zimPath);

    if (args.at("--suggestion").asBool()) {
      zim::SuggestionSearcher searcher(archive);
      searcher.setVerbose(verbose);
      for (const auto& r : searcher.suggest(pattern).getResults(0, 10)) {
        std::cout << r.getTitle() << std::endl;
      }
    } else if (args.at("--spelling").asBool()) {
      auto cacheDir = getCacheDir();
      std::filesystem::create_directories(cacheDir);
      kiwix::SpellingsDB spellingsDB(archive, cacheDir);
      for (const auto& r : spellingsDB.getSpellingCorrections(pattern, 1)) {
        std::cout << r << std::endl;
      }
    } else {
      zim::Searcher searcher(archive);
      searcher.setVerbose(verbose);
      const zim::Query query(pattern);
      for (const auto& r : searcher.search(query).getResults(0, 10)) {
        std::cout << r.getTitle() << std::endl;
      }
    }
  } catch (const std::runtime_error& err) {
    std::cerr << err.what() << std::endl;
    return 1;
  } catch (const Xapian::Error& err) {
    std::cerr << err.get_msg() << std::endl;
    return 1;
  }

  return 0;
}
