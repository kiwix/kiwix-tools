/*
 * Copyright 2012-2014 
 * Renaud Gaudin <reg@kiwix.org>
 * Emmanuel Engelhart <kelson@kiwix.org>
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

#include <stdlib.h>
#include <unistd.h>

#include "pathTools.h"

using namespace std;

int main(int argc, char *argv[]) {
    
    /* Initialisation of a few paths */
    string executablePath = getExecutablePath();
    string executableDirectory = removeLastPathElement(executablePath);
    string applicationIniPath = computeAbsolutePath(executableDirectory, "application.ini");

    /* Find xulrunner directory */
    string xulrunnerDirectory = computeAbsolutePath(executableDirectory, "xulrunner");
    if (!fileExists(xulrunnerDirectory)) {
      xulrunnerDirectory = computeAbsolutePath(executableDirectory, "kiwix");
      xulrunnerDirectory = computeAbsolutePath(executableDirectory, "xulrunner");
      if (!fileExists(xulrunnerDirectory)) {
	perror("Unable to find the xulrunner directory");
	return EXIT_FAILURE;
      }
    }

    /* Find xulrunner binary path */
    string xulrunnerPath = computeAbsolutePath(xulrunnerDirectory, "xulrunner-bin");
    if (!fileExists(xulrunnerPath)) {
      xulrunnerPath = computeAbsolutePath(xulrunnerDirectory, "xulrunner");
      if (!fileExists(xulrunnerPath)) {
	perror("Unable to find neither the 'xulrunner-bin' nor the 'xulrunner' binary");
	return EXIT_FAILURE;
      }
    }

    /* Debug prints */
    /*
    cout  << "Executable directory (executableDirectory): " << executableDirectory << endl;
    cout  << "Executable path (executablePath): " << executablePath << endl;
    cout  << "Xulrunner directory (xulrunnerDirectory): " << xulrunnerDirectory << endl;
    cout  << "Xulrunner path (xulrunnerPath): " << xulrunnerPath << endl;
    cout  << "Application.ini path (applicationIniPath): " << applicationIniPath << endl;
    */

    /* Modify environnement variables */
#ifdef _WIN32
    string sep = ";";
#else
    string sep = ":";
#endif
    string ldLibraryPath = getenv("LD_LIBRARY_PATH") == NULL ? "" : string(getenv("LD_LIBRARY_PATH"));
    string putenvStr = "LD_LIBRARY_PATH=" + xulrunnerDirectory + sep + ldLibraryPath;
    putenv((char *)putenvStr.c_str());

    /* Launch xulrunner */
    if (argc == 0) {
      return execl(xulrunnerPath.c_str(), xulrunnerPath.c_str(), applicationIniPath.c_str(), 
		   "", NULL);
    } else if (argc == 1) {
      return execl(xulrunnerPath.c_str(), xulrunnerPath.c_str(), applicationIniPath.c_str(), 
		   argv[1], NULL);
    } else if (argc == 2) {
      return execl(xulrunnerPath.c_str(), xulrunnerPath.c_str(), applicationIniPath.c_str(), 
		   argv[1], argv[2], NULL);
    } else if (argc == 3) {
      return execl(xulrunnerPath.c_str(), xulrunnerPath.c_str(), applicationIniPath.c_str(), 
		   argv[1], argv[2], argv[3], NULL);
    } else if (argc == 4) {
      return execl(xulrunnerPath.c_str(), xulrunnerPath.c_str(), applicationIniPath.c_str(), 
		   argv[1], argv[2], argv[3], argv[4], NULL);
    } else if (argc == 5) {
      return execl(xulrunnerPath.c_str(), xulrunnerPath.c_str(), applicationIniPath.c_str(), 
		   argv[1], argv[2], argv[3], argv[4], argv[5], NULL);
    } else if (argc == 6) {
      return execl(xulrunnerPath.c_str(), xulrunnerPath.c_str(), applicationIniPath.c_str(),
		   argv[1], argv[2], argv[3], argv[4], argv[5], argv[6], NULL);
    } else if (argc == 7) {
      return execl(xulrunnerPath.c_str(), xulrunnerPath.c_str(), applicationIniPath.c_str(), 
		   argv[1], argv[2], argv[3], argv[4], argv[5], argv[6], argv[7], NULL);
    } else if (argc == 8) {
      return execl(xulrunnerPath.c_str(), xulrunnerPath.c_str(), applicationIniPath.c_str(), 
		   argv[1], argv[2], argv[3], argv[4], argv[5], argv[6], argv[7], argv[8], NULL);
    } else if (argc >= 9) {
      if (argc > 9) {
	fprintf(stderr, "Kiwix was not able to forward all your arguments to the xulrunner binary.");
      }
      return execl(xulrunnerPath.c_str(), xulrunnerPath.c_str(), applicationIniPath.c_str(), 
		   argv[1], argv[2], argv[3], argv[4], argv[5], argv[6], argv[7], argv[8], argv[9], NULL);
    }
}
