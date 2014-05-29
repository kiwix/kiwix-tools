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

#ifdef _WIN32
#pragma comment( linker, "/subsystem:\"windows\" /entry:\"mainCRTStartup\"" )
#include <Windows.h>
#include <process.h>
#define EXECL _execl
#define PUTENV _putenv
#else
#include <stdlib.h>
#include <unistd.h>
#define EXECL execl
#define PUTENV putenv
#endif

#include <vector>
#include "pathTools.h"

using namespace std;

/* Split string in a token array */
std::vector<std::string> split(const std::string & str,
			       const std::string & delims=" *-")
{
  std::string::size_type lastPos = str.find_first_not_of(delims, 0);
  std::string::size_type pos = str.find_first_of(delims, lastPos);
  std::vector<std::string> tokens;

  while (std::string::npos != pos || std::string::npos != lastPos)
    {
      tokens.push_back(str.substr(lastPos, pos - lastPos));
      lastPos = str.find_first_not_of(delims, pos);
      pos     = str.find_first_of(delims, lastPos);
    }

  return tokens;
}

/* Quote string on Windows */
char *prepareArgument(const char *argument) {
  if (argument != NULL) {
#ifdef _WIN32
    string quotedArgument = "\"" + string(argument) + "\"";
    return strdup(quotedArgument.c_str());
#else
    return strdup(argument);
#endif
  } else {
    return NULL;
  }
}

int main(int argc, char *argv[]) {

    /* Initialisation of a few paths */
    string executablePath = getExecutablePath();
    string executableDirectory = removeLastPathElement(executablePath);
    
    /* Possible xulrunner paths */ 
    std::vector<std::string> xulrunnerPossibleDirectories;
    std::vector<std::string>::iterator directoriesIt;

    /* Possible xulrunner paths: local directories */
    xulrunnerPossibleDirectories.push_back(computeAbsolutePath(executableDirectory, "xulrunner"));
    xulrunnerPossibleDirectories.push_back(computeAbsolutePath(executableDirectory, "kiwix/xulrunner"));
    xulrunnerPossibleDirectories.push_back(computeAbsolutePath(executableDirectory, "kiwix-linux/xulrunner"));
    xulrunnerPossibleDirectories.push_back(computeAbsolutePath(executableDirectory, "kiwix-win/xulrunner"));
    xulrunnerPossibleDirectories.push_back(computeAbsolutePath(executableDirectory, "kiwix-windows/xulrunner"));

    /* Possible xulrunner paths: system directories */
    string binaryPath = getenv("PATH") == NULL ? "" : string(getenv("PATH"));
    std::vector<std::string> xulrunnerPossibleSystemDirectories = ::split(binaryPath, ":");
    for (directoriesIt = xulrunnerPossibleSystemDirectories.begin() ;
	 directoriesIt != xulrunnerPossibleSystemDirectories.end() ;
	 directoriesIt++) {
      xulrunnerPossibleDirectories.push_back(*directoriesIt);
    }

    /* Find xulrunner (binary) path */
    string xulrunnerPath;
    directoriesIt = xulrunnerPossibleDirectories.begin();
    while (xulrunnerPath.empty() && directoriesIt != xulrunnerPossibleDirectories.end()) {
      if (fileExists(*directoriesIt)) {
#ifdef _WIN32
	xulrunnerPath = computeAbsolutePath(*directoriesIt, "xulrunner-bin.exe");
#else
	xulrunnerPath = computeAbsolutePath(*directoriesIt, "xulrunner-bin");
#endif

	if (!fileExists(xulrunnerPath)) {
#ifdef _WIN32
	  xulrunnerPath = computeAbsolutePath(*directoriesIt, "xulrunner.exe");
#else
	  xulrunnerPath = computeAbsolutePath(*directoriesIt, "xulrunner");
#endif
	  if (!fileExists(xulrunnerPath)) {
	    xulrunnerPath.clear();
	  }
	}
      }
      directoriesIt++;
    }    
    if (!fileExists(xulrunnerPath)) {
      perror("Unable to find neither the 'xulrunner-bin' nor the 'xulrunner' binary");
      return EXIT_FAILURE;
    }

    /* Compute xulrunner directory */
    string xulrunnerDirectory = removeLastPathElement(xulrunnerPath); 

    /* Compute application.ini path */
    std::vector<std::string> applicationIniPossiblePaths;
    applicationIniPossiblePaths.push_back(computeAbsolutePath(executableDirectory, 
							      "application.ini"));
    applicationIniPossiblePaths.push_back(computeAbsolutePath(executableDirectory, 
							      "kiwix/application.ini"));
    applicationIniPossiblePaths.push_back(computeAbsolutePath(executableDirectory, 
							      "kiwix-linux/application.ini"));
    applicationIniPossiblePaths.push_back(computeAbsolutePath(executableDirectory, 
							      "kiwix-win/application.ini"));
    applicationIniPossiblePaths.push_back(computeAbsolutePath(executableDirectory, 
							      "kiwix-windows/application.ini"));

    string applicationIniPath;
    std::vector<std::string>::iterator filesIt = applicationIniPossiblePaths.begin();
    while (applicationIniPath.empty() && 
	   filesIt != applicationIniPossiblePaths.end()) {
      if (fileExists(*filesIt)) {
	applicationIniPath = *filesIt;
      }
      filesIt++;
    };
    if (!fileExists(xulrunnerPath)) {
      perror("Unable to find the application.ini file");
      return EXIT_FAILURE;
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
      return EXECL(xulrunnerPath.c_str(), 
		   execlArg0.c_str(), 
		   prepareArgument(applicationIniPath.c_str()),
		   NULL);
    } else if (argc == 1) {
      return EXECL(xulrunnerPath.c_str(), 
		   execlArg0.c_str(), 
		   prepareArgument(applicationIniPath.c_str()), 
		   prepareArgument(argv[1]),
		   NULL);
    } else if (argc == 2) {
      return EXECL(xulrunnerPath.c_str(), 
		   execlArg0.c_str(), 
		   prepareArgument(applicationIniPath.c_str()), 
		   prepareArgument(argv[1]),
		   prepareArgument(argv[2]),
		   NULL);
    } else if (argc == 3) {
      return EXECL(xulrunnerPath.c_str(), 
		   execlArg0.c_str(), 
		   prepareArgument(applicationIniPath.c_str()), 
		   prepareArgument(argv[1]),
		   prepareArgument(argv[2]),
		   prepareArgument(argv[3]),
		   NULL);
    } else if (argc == 4) {
      return EXECL(xulrunnerPath.c_str(), 
		   execlArg0.c_str(),
		   prepareArgument(applicationIniPath.c_str()), 
		   prepareArgument(argv[1]),
		   prepareArgument(argv[2]),
		   prepareArgument(argv[3]),
		   prepareArgument(argv[4]),
		   NULL);
    } else if (argc == 5) {
      return EXECL(xulrunnerPath.c_str(), 
		   execlArg0.c_str(),
		   prepareArgument(applicationIniPath.c_str()), 
		   prepareArgument(argv[1]),
		   prepareArgument(argv[2]),
		   prepareArgument(argv[3]),
		   prepareArgument(argv[4]),
		   prepareArgument(argv[5]),
		   NULL);
    } else if (argc == 6) {
      return EXECL(xulrunnerPath.c_str(), 
		   execlArg0.c_str(),
		   prepareArgument(applicationIniPath.c_str()),
		   prepareArgument(argv[1]),
		   prepareArgument(argv[2]),
		   prepareArgument(argv[3]),
		   prepareArgument(argv[4]),
		   prepareArgument(argv[5]),
		   prepareArgument(argv[6]),
		   NULL);
    } else if (argc == 7) {
      return EXECL(xulrunnerPath.c_str(), 
		   execlArg0.c_str(),
		   prepareArgument(applicationIniPath.c_str()), 
		   prepareArgument(argv[1]),
		   prepareArgument(argv[2]),
		   prepareArgument(argv[3]),
		   prepareArgument(argv[4]),
		   prepareArgument(argv[5]),
		   prepareArgument(argv[6]),
		   prepareArgument(argv[7]),
		   NULL);
    } else if (argc == 8) {
      return EXECL(xulrunnerPath.c_str(), 
		   execlArg0.c_str(),
		   prepareArgument(applicationIniPath.c_str()), 
		   prepareArgument(argv[1]),
		   prepareArgument(argv[2]),
		   prepareArgument(argv[3]),
		   prepareArgument(argv[4]),
		   prepareArgument(argv[5]),
		   prepareArgument(argv[6]),
		   prepareArgument(argv[7]),
		   prepareArgument(argv[8]),
		   NULL);
    } else if (argc >= 9) {
      if (argc > 9) {
	cerr << "Kiwix was not able to forward all your arguments to the xulrunner binary." << endl;
      }
      return EXECL(xulrunnerPath.c_str(), 
		   execlArg0.c_str(),
		   prepareArgument(applicationIniPath.c_str()), 
		   prepareArgument(argv[1]),
		   prepareArgument(argv[2]),
		   prepareArgument(argv[3]),
		   prepareArgument(argv[4]),
		   prepareArgument(argv[5]),
		   prepareArgument(argv[6]),
		   prepareArgument(argv[7]),
		   prepareArgument(argv[8]),
		   prepareArgument(argv[9]),
		   NULL);
    }
}
