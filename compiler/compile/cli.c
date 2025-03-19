#include <stdio.h>
#include <string.h>

#include "compile.h"

void help_cli() {
    printf("Tasia is literally the new rust\n");
    printf("use it or you're not a real programmer");
}

int cli(char* command, char* filepath) {

    if (strcmp(command, "build") == 0) {
        compile(filepath);
        return 0;

    } else if (strcmp(command, "help") == 0) {
        help_cli();
        return 0;
        
    } else {
        printf("the cli command %s is invalid", command);
        return 1;
    }

}