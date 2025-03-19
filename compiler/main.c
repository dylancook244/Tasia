#include <stdio.h>

#ifdef _WIN32
    #include <direct.h>
    #define getcwd _getcwd // stupid MSFT "deprecation" warning
#else
    #include <unistd.h>
#endif

#include "compile/compile.h"

char* get_full_path(char* filename) {
    // allocate space on stack for current working directory
    static char cwd[1024];

    // allocate space on stack for full filepath
    static char filepath[1024];

    // gets cwd and returns an error if we can't access it
    if (getcwd(cwd, sizeof(cwd)) == NULL) {
        printf("getcwd error, couldn't access current working directory\n");
        return NULL;
    }

    // apparently sprintf doesn't actually print
    // it's what allocates stack memory. weird
    sprintf(filepath, "%s/%s", cwd, filename);

    return filepath;
}

int main(int argc, char** argv) {

    if (argc < 2) {
        printf("please enter an argument for tasia\n");
        printf("Usage: tasia <command> <file>");
        return 1;
    }

    // there should ALWAYS be a command, it will be the 2nd argument
    char* command = argv[1];

    // 3rd arg ALWAYS filename
    if (argc == 3) {
        char* filename = argv[2];
        char* filepath = get_full_path(filename);
        if (cli(command, filepath) == 1) {
            return 1;
        };
    } else {
        char* filepath = NULL;
        if (cli(command, filepath) == 1) {
            return 1;
        };
    }

    return 0;

}