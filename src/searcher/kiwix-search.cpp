/*
 * Copyright 2011 Emmanuel Engelhart <kelson@kiwix.org>
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

#include <zim/search.h>
#include <zim/suggestion.h>

#include <kiwix/spelling_correction.h>
#include <xapian.h>

#include <iostream>
#include <filesystem>

#include "../version.h"

using namespace std;


// Older version of docopt doesn't declare Options. Let's declare it ourself.
using Options = std::map<std::string, docopt::value>;

static const char USAGE[] =
R"(Find articles based on a fulltext search pattern.

Usage:
  kiwix-search [options] ZIM PATTERN
  kiwix-search -h | --help
  kiwix-search -V | --version

Arguments:
  ZIM       The full path of the ZIM file
  PATTERN   Word(s) - or part of - to search in the ZIM.

Options:
  -s --suggestion    Suggest article titles based on the few letters of the PATTERN instead of making a fulltext search. Work a bit like a completion solution
  --spelling         Suggest article titles based on the spelling corrected PATTERN instead of making a fulltext search.
  -v --verbose       Give details about the search process
  -V --version       Print software version
  -h --help          Print this help
)";

std::filesystem::path getKiwixCachedDataDirPath()
{
  std::filesystem::path home(getenv("HOME"));
  std::filesystem::path cacheDirPath = home / ".cache" / "kiwix";
  std::filesystem::create_directories(cacheDirPath);
  return cacheDirPath;
}

int main(int argc, char** argv)
{
  Options args;
  try {
    args = docopt::docopt_parse(USAGE, {argv+1, argv+argc}, false, false);
  } catch (docopt::DocoptArgumentError const & error ) {
    std::cerr << error.what() << std::endl;
    std::cerr << USAGE << std::endl;
    return -1;
  }

  if (args.at("--help").asBool()) {
    std::cout << USAGE << std::endl;
    return 0;
  }

  if (args.at("--version").asBool()) {
    version();
    return 0;
  }

  auto zimPath = args.at("ZIM").asString();
  auto pattern = args.at("PATTERN").asString();
  auto verboseFlag = args.at("--verbose").asBool();

  /* Try to prepare the indexing */
  try {
    zim::Archive archive(zimPath);

    if (args.at("--suggestion").asBool()) {
      zim::SuggestionSearcher searcher(archive);
      searcher.setVerbose(verboseFlag);
      for (const auto& r:searcher.suggest(pattern).getResults(0, 10)) {
        cout << r.getTitle() << endl;
      }
    }  else if (args.at("--spelling").asBool()) {
      kiwix::SpellingsDB spellingsDB(archive, getKiwixCachedDataDirPath());
      for (const auto& r:spellingsDB.getSpellingCorrections(pattern, 1)) {
        cout << r << endl;
      }
    } else {
      zim::Searcher searcher(archive);
      searcher.setVerbose(verboseFlag);
      const zim::Query query(pattern);
      for (const auto& r : searcher.search(query).getResults(0, 10) ) {
        cout << r.getTitle() << endl;
      }
    }
  } catch ( const std::runtime_error& err)  {
    cerr << err.what() << endl;
    exit(1);
  } catch ( const Xapian::Error& err)  {
    cerr << err.get_msg() << endl;
    exit(1);
  }

  exit(0);
}
