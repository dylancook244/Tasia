#include <stdio.h>
#include "parser.h"
#include "../scanner/scanner.h"

void parseProgram(Scanner* scanner) {
    printf("\n\n=== SOURCE CODE ===\n");

    // Reset the scanner to the beginning of the file
    rewind(scanner->file);
    // Re-initialize scanner's current_char
    int c = fgetc(scanner->file);
    scanner->current_char = (c == EOF) ? '\1' : (char)c;
    scanner->line = 1;
    scanner->column = 1;

    // Print the source code
    while (scanner->current_char != '\1') {  // Changed '\0' to '\1'
        printf("%c", scanner->current_char);
        advance_char(scanner);
    }

    printf("\n\n=== TOKEN DUMP ===\n");
    
    // Reset the scanner again to tokenize from the beginning
    // scanner already passed through in the SOURCE CODE output
    rewind(scanner->file);
    // Re-initialize scanner's current_char
    c = fgetc(scanner->file);
    scanner->current_char = (c == EOF) ? '\1' : (char)c;
    scanner->line = 1;
    scanner->column = 1;
    
    // Get and process tokens
    Token* token = getNextToken(scanner);

    // TODO: doesn't actually print end of file, just detects it
    // make it actually print the EOF in debug
    while (token->type != END_OF_FILE) {
        // Process token
        printf("\nToken: %s, raw_text: '%s', Line: %d, Col: %d", 
               token_type_to_string(token->type), 
               token->raw_text, 
               token->line, 
               token->column);
        
        // Free the token
        free_token(token);
        
        // Get next token
        token = getNextToken(scanner);
    }
    
    // Don't forget to free the final EOF token
    free_token(token);
}