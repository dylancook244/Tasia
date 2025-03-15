#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>

#include "scanner.h"

Scanner* init_scanner(const char* filepath) {
    Scanner* scanner = malloc(sizeof(Scanner));
    if (!scanner) return NULL;

    scanner->file = fopen(filepath, "r");
    if (!scanner->file) {
        free(scanner);
        return NULL;
    }

    scanner->line = 1;
    scanner->column = 0;
    scanner->filename = filepath;

    // get first char ready
    int c = fgetc(scanner->file);
    scanner->current_char = (c == EOF) ? '\0' : (char)c;
    scanner->column = 1;

    return scanner;
}

// advances to next char in a file
// returns char for convenience, 
// in case we want to return the value we just advanced to
char advance_char(Scanner* scanner) {
    if (scanner->current_char == '\n') {
        scanner->line++;
        scanner->column = 0;
    }

    int c = fgetc(scanner->file);
    scanner->current_char = (c == EOF) ? '\0' : (char)c;
    scanner->column++;

    return scanner->current_char;
}

// free the scanner and file we're scanning
void close_scanner(Scanner* scanner) {
    if (scanner) {
        if (scanner->file) fclose(scanner->file);
        free(scanner);
    }
}