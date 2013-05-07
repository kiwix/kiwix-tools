
#include <stdlib.h>
#include <unistd.h>

#include "pathTools.h"

using namespace std;

int main(int argc, char *argv[])
{
    
    // find current binary path
    string progpath = getExecutablePath();
    string cwd = removeLastPathElement(progpath);
    string ld_path = computeAbsolutePath(cwd, "xulrunner");
    
    // retrieve existing env if exist.
    string previous_env;
    char *previous_env_buf = getenv("LD_LIBRARY_PATH");
    if (previous_env_buf == NULL)
        previous_env = "";
    else
        previous_env = string(previous_env_buf);

#ifdef _WIN32
    string sep = ";";
#else
    string sep = ":";
#endif

    // generate putenv string
    string env_str = "LD_LIBRARY_PATH=" + ld_path + sep + previous_env;
    putenv((char *)env_str.c_str());

    string xulrunner_path = computeAbsolutePath(cwd, "xulrunner/xulrunner-bin");
    string application_ini = computeAbsolutePath(cwd, "application.ini");

    //debug prints
    /*
    cout  << "CurProg: " << progpath << endl;
    cout  << "cwd: " << cwd << endl;
    cout  << "LD path: " << ld_path << endl;
    cout  << "xulrunner_path: " << xulrunner_path << endl;
    cout  << "application_ini: " << application_ini << endl;
    */

    // exist if xulrunner can't be found
    if (!fileExists(xulrunner_path)) {
        xulrunner_path = computeAbsolutePath(cwd, "xulrunner/xulrunner");
        if (!fileExists(xulrunner_path)) {
            perror("Unable to find neither the 'xulrunner-bin' nor the 'xulrunner' binary");
            return EXIT_FAILURE;
        }
    }

    // execute xulrunner
    if (argc == 0) {
      return execl(xulrunner_path.c_str(), xulrunner_path.c_str(), application_ini.c_str(), 
		   "", NULL);
    } else if (argc == 1) {
      return execl(xulrunner_path.c_str(), xulrunner_path.c_str(), application_ini.c_str(), 
		   argv[1], NULL);
    } else if (argc == 2) {
      return execl(xulrunner_path.c_str(), xulrunner_path.c_str(), application_ini.c_str(), 
		   argv[1], argv[2], NULL);
    } else if (argc == 3) {
      return execl(xulrunner_path.c_str(), xulrunner_path.c_str(), application_ini.c_str(), 
		   argv[1], argv[2], argv[3], NULL);
    } else if (argc == 4) {
      return execl(xulrunner_path.c_str(), xulrunner_path.c_str(), application_ini.c_str(), 
		   argv[1], argv[2], argv[3], argv[4], NULL);
    } else if (argc == 5) {
      return execl(xulrunner_path.c_str(), xulrunner_path.c_str(), application_ini.c_str(), 
		   argv[1], argv[2], argv[3], argv[4], argv[5], NULL);
    } else if (argc == 6) {
      return execl(xulrunner_path.c_str(), xulrunner_path.c_str(), application_ini.c_str(),
		   argv[1], argv[2], argv[3], argv[4], argv[5], argv[6], NULL);
    } else if (argc == 7) {
      return execl(xulrunner_path.c_str(), xulrunner_path.c_str(), application_ini.c_str(), 
		   argv[1], argv[2], argv[3], argv[4], argv[5], argv[6], argv[7], NULL);
    } else if (argc == 8) {
      return execl(xulrunner_path.c_str(), xulrunner_path.c_str(), application_ini.c_str(), 
		   argv[1], argv[2], argv[3], argv[4], argv[5], argv[6], argv[7], argv[8], NULL);
    } else if (argc >= 9) {
      if (argc>9) {
	fprintf(stderr, "Kiwix was not able to forward all your arguments to the xulrunner binary.");
      }
      return execl(xulrunner_path.c_str(), xulrunner_path.c_str(), application_ini.c_str(), 
		   argv[1], argv[2], argv[3], argv[4], argv[5], argv[6], argv[7], argv[8], argv[9], NULL);
    }
}
