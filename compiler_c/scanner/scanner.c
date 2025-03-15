#include <stdlib.h>
#include <ctype.h>
#include <string.h>

#include "scanner.h"
#include "token.h"

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
    scanner->current_char = (c == EOF) ? '\1' : (char)c;
    scanner->column++;

    return scanner->current_char;
}

Token* getNextToken(Scanner* scanner) {
    // skip spaces, spaces not included
    while (isspace(scanner->current_char) && scanner->current_char != '\n') {
        advance_char(scanner);
    }

    // Create token
    Token* token = malloc(sizeof(Token));
    if (!token) return NULL;
    
    token->line = scanner->line;
    token->column = scanner->column;
    token->raw_text = NULL;

    // Check for end of line
    //if ()
    
    // Check for end of file
    if (scanner->current_char == '\1') {
        token->type = END_OF_FILE;
        token->raw_text = my_strdup("EOF");
        return token;
    }

    // if we get here, it's not a token
    token->type = ILLEGAL;
    char raw_text[2] = {scanner->current_char, '\0'};
    token->raw_text = strdup(raw_text);
    advance_char(scanner);
    return token;
}

// free the scanner and file we're scanning
void free_scanner(Scanner* scanner) {
    if (scanner) {
        // fclose because it's a file
        if (scanner->file) fclose(scanner->file);
        free(scanner);
    }
}