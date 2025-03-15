#ifndef SCANNER_H
#define SCANNER_H

#include <stdio.h>
#include "token.h"

// struct for our scanner
typedef struct {
    FILE* file;
    char current_char;
    int line;
    int column;
    const char* filename;
} Scanner;

// make the scanner
Scanner* init_scanner(const char* filepath);

// advance characters
char advance_char(Scanner* scanner);

// get next token
Token* getNextToken(Scanner* scanner);

// Scanner is heap allocated
void free_scanner(Scanner* scanner);

#endif
