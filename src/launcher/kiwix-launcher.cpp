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


#ifdef _WIN32
#pragma comment( linker, "/subsystem:\"windows\" /entry:\"mainCRTStartup\"" )
#include <Windows.h>
#include <process.h>
#define EXECL _execl
#define PUTENV _putenv
#else
#include <unistd.h>
#define EXECL execl
#define PUTENV putenv
#endif

#include <vector>

#include "pathTools.h"

using namespace std;

int main(int argc, char *argv[]) {

    /* Initialisation of a few paths */
    string executablePath = getExecutablePath();
    string executableDirectory = removeLastPathElement(executablePath);
    
    /* Possible xulrunner paths */ 
    std::vector<std::string> xulrunnerPossibleDirectories;
    xulrunnerPossibleDirectories.push_back("xulrunner");
    xulrunnerPossibleDirectories.push_back("kiwix/xulrunner");
    xulrunnerPossibleDirectories.push_back("kiwix-linux/xulrunner");
    xulrunnerPossibleDirectories.push_back("kiwix-win/xulrunner");
    xulrunnerPossibleDirectories.push_back("kiwix-windows/xulrunner");

    /* Find xulrunner directory */
    string xulrunnerDirectory;
    std::vector<std::string>::iterator directoriesIt = xulrunnerPossibleDirectories.begin();
    while (xulrunnerDirectory.empty() && directoriesIt != xulrunnerPossibleDirectories.end()) {
      xulrunnerDirectory = computeAbsolutePath(executableDirectory, *directoriesIt);
      if (!fileExists(xulrunnerDirectory)) {
	xulrunnerDirectory.clear();
	directoriesIt++;
      }
    }    
    if (!fileExists(xulrunnerDirectory)) {
        perror("Unable to find the xulrunner directory");
	return EXIT_FAILURE;
    }

    /* Find xulrunner binary path */
#ifdef _WIN32
    string xulrunnerPath = computeAbsolutePath(xulrunnerDirectory, "xulrunner-bin.exe");
#else
    string xulrunnerPath = computeAbsolutePath(xulrunnerDirectory, "xulrunner-bin");
#endif
    if (!fileExists(xulrunnerPath)) {
#ifdef _WIN32
      xulrunnerPath = computeAbsolutePath(xulrunnerDirectory, "xulrunner.exe");
#else
      xulrunnerPath = computeAbsolutePath(xulrunnerDirectory, "xulrunner");
#endif
      if (!fileExists(xulrunnerPath)) {
	perror("Unable to find neither the 'xulrunner-bin' nor the 'xulrunner' binary");
	return EXIT_FAILURE;
      }
    }

    /* Compute application.ini path */
    string applicationIniPath = computeAbsolutePath(removeLastPathElement(xulrunnerDirectory, false, false), "application.ini");
#ifdef _WIN32
    applicationIniPath = "\"" + applicationIniPath + "\"";
#endif    

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
    string execlArg0 = "kiwix-launcher.exe";
#else
    string sep = ":";
    string execlArg0 = xulrunnerPath.c_str();
#endif

    string ldLibraryPath = getenv("LD_LIBRARY_PATH") == NULL ? "" : string(getenv("LD_LIBRARY_PATH"));
    string putenvStr = "LD_LIBRARY_PATH=" + xulrunnerDirectory + sep + ldLibraryPath;
    PUTENV((char *)putenvStr.c_str());

    /* Launch xulrunner */
    if (argc == 0) {
      return EXECL(xulrunnerPath.c_str(), execlArg0.c_str(), applicationIniPath.c_str(), 
		   "", NULL);
    } else if (argc == 1) {
      return EXECL(xulrunnerPath.c_str(), execlArg0.c_str(), applicationIniPath.c_str(), 
		   argv[1], NULL);
    } else if (argc == 2) {
      return EXECL(xulrunnerPath.c_str(), execlArg0.c_str(), applicationIniPath.c_str(), 
		   argv[1], argv[2], NULL);
    } else if (argc == 3) {
      return EXECL(xulrunnerPath.c_str(), execlArg0.c_str(), applicationIniPath.c_str(), 
		   argv[1], argv[2], argv[3], NULL);
    } else if (argc == 4) {
      return EXECL(xulrunnerPath.c_str(), execlArg0.c_str(), applicationIniPath.c_str(), 
		   argv[1], argv[2], argv[3], argv[4], NULL);
    } else if (argc == 5) {
      return EXECL(xulrunnerPath.c_str(), execlArg0.c_str(), applicationIniPath.c_str(), 
		   argv[1], argv[2], argv[3], argv[4], argv[5], NULL);
    } else if (argc == 6) {
      return EXECL(xulrunnerPath.c_str(), execlArg0.c_str(), applicationIniPath.c_str(),
		   argv[1], argv[2], argv[3], argv[4], argv[5], argv[6], NULL);
    } else if (argc == 7) {
      return EXECL(xulrunnerPath.c_str(), execlArg0.c_str(), applicationIniPath.c_str(), 
		   argv[1], argv[2], argv[3], argv[4], argv[5], argv[6], argv[7], NULL);
    } else if (argc == 8) {
      return EXECL(xulrunnerPath.c_str(), execlArg0.c_str(), applicationIniPath.c_str(), 
		   argv[1], argv[2], argv[3], argv[4], argv[5], argv[6], argv[7], argv[8], NULL);
    } else if (argc >= 9) {
      if (argc > 9) {
	fprintf(stderr, "Kiwix was not able to forward all your arguments to the xulrunner binary.");
      }
      return EXECL(xulrunnerPath.c_str(), execlArg0.c_str(), applicationIniPath.c_str(), 
		   argv[1], argv[2], argv[3], argv[4], argv[5], argv[6], argv[7], argv[8], argv[9], NULL);
    }
}
