
#include <unistd.h>
#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

#ifdef _WIN32
const char * sep = "\\";
#else
const char * sep = "/";
#endif

int main() {

	char cwd[1024]; 

	if (getcwd(cwd, sizeof(cwd)) != NULL) {
        char env[1024] = "LD_LIBRARY_PATH=";
        strcat(env, cwd);
        putenv(env);
        // fprintf(stdout, "Current working dir: %s\n", cwd);
    } else
		perror("Unable to find current directory");

    char *path = (char *)malloc(2048);
    strcpy(path, cwd);
    strcat(path, sep);
    strcat(path, "xulrunner");
    strcat(path, sep);
    strcat(path, "xulrunner");
    // fprintf(stdout, "path: %s\n", path);

    char *inifile = (char *)malloc(2048);
    strcpy(inifile, cwd);
    strcat(inifile, sep);
    strcat(inifile, "application.ini");
    // fprintf(stdout, "ini: %s\n", inifile);

    
    if (file_exist(path) == 0) {
        perror("Unable to find xulrunner binary");
    }

    return execl(path, "xulrunner", inifile, NULL);
}

int file_exist (char *filename) {
    struct stat   buffer;   
    return (stat (filename, &buffer) == 0);
}
