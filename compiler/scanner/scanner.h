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
    
    // collects stream of tokens
    Token** tokens;
    int capacity; // size of array
    int count; // num of tokens in array
    int position; // position we're at in array
} Scanner;

// make the scanner
Scanner* create_scanner(const char* filepath);

// advance characters
char advance_char(Scanner* scanner);

// tokenize
Token* tokenize(Scanner* scanner);

// get next token
// sometimes we need a chunk of tokens so we increment without the
// pointer and then return back to the pointer
Token* getNextToken(Scanner* scanner);

// delete current token, reset state as if that token never happened
void goBackToken(Scanner* scanner); 

// Scanner is heap allocated
void free_scanner(Scanner* scanner);

#endif
