
#include <stdlib.h>

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

    string xulrunner_path = computeAbsolutePath(cwd, "xulrunner/xulrunner");
    string application_ini = computeAbsolutePath(cwd, "application.ini");

    // debug prints
    // cout  << "CurProg: " << progpath << endl;
    // cout  << "cwd: " << cwd << endl;
    // cout  << "LD path: " << ld_path << endl;
    // cout  << "xulrunner_path: " << xulrunner_path << endl;
    // cout  << "application_ini: " << application_ini << endl;

    // exist if xulrunner can't be found
    if (!fileExists(xulrunner_path)) {
        perror("Unable to find xulrunner binary");
        return EXIT_FAILURE;
    }

    // forward extra argument
    char *argument = "";
    if (argc > 0) {
        argument = argv[1];
    }

    // execute xulrunner
    return execl(xulrunner_path.c_str(), "xulrunner", application_ini.c_str(), argument, NULL);
}