#ifndef SCANNER_H
#define SCANNER_H

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

// Scanner is heap allocated
void close_scanner(Scanner* scanner);

#endif
