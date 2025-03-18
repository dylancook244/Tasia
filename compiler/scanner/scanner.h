#ifndef SCANNER_H
#define SCANNER_H

#include <stdio.h>
#include "token.h"

typedef struct {
    FILE* file;
    char current_char;
    int line;
    int column;
    const char* filename;
    
    bool has_buffered_token;
    Token* buffered_token;
} Scanner;

// make the scanner
Scanner* init_scanner(const char* filepath);

// advance characters
char advance_char(Scanner* scanner);

// get next token
Token* getNextToken(Scanner* scanner);

// unget token
static void ungetToken(Scanner* scanner, Token* token);

// Scanner is heap allocated
void free_scanner(Scanner* scanner);

#endif
