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

#include <iostream>
#include <cstdlib>
#include <kiwix/manager.h>

using namespace std;

enum supportedAction { NONE, ADD, SHOW, REMOVE };

int main(int argc, char **argv) {

  string libraryPath = "";
  supportedAction action = NONE;
  string zimPath = "";
  kiwix::Manager libraryManager;
  
  /* Argument parsing */
  if (argc > 2) {
    libraryPath = argv[1];
    string actionString = argv[2];

    if (actionString == "add")
      action = ADD;
    else if (actionString == "show")
      action = SHOW;
    else if (actionString == "remove" || actionString == "delete")
      action = REMOVE;
  }
  
  /* Print usage)) if necessary */
  if (libraryPath == "" || action == NONE) {
    cerr << "Usage: kiwix-manage LIBRARY_PATH ACTION [OPTIONS]" << endl;
    exit(1);
  }

  /* SHOW */
  if (action == SHOW) {
    if (!libraryManager.readFile(libraryPath)) {
      std::cerr << "Unable to read the library file '" << libraryPath << "'." << std::endl;
      exit(1);
    }
  } else if (action == ADD) {
    std::cerr << "ADD is still not implemented." << std::endl;
  } else if (action == REMOVE) {
    std::cerr << "REMOVE is still not implemented." << std::endl;
  }

  exit(0);
}
