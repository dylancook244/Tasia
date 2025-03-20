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

    scanner->capacity = 16;
    scanner->tokens = malloc(sizeof(Token*) * scanner->capacity);

    scanner->count = 0;
    scanner->position = 0;

    scanner->line = 1;
    scanner->column = 0;
    scanner->filename = filepath;

    // get first char ready
    int c = fgetc(scanner->file);
    scanner->current_char = (c == EOF) ? '\0' : (char)c;
    scanner->column = 1;

    return scanner;
}

// Use this instead of strdup
static char* my_strdup(const char* str) {
    size_t len = strlen(str) + 1;
    char* new_str = malloc(len);
    if (new_str) {
        memcpy(new_str, str, len);
    }
    return new_str;
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

Token* tokenize(Scanner* scanner) {
    // skip spaces, spaces not included
    while (isspace(scanner->current_char) && scanner->current_char != '\n') {
        advance_char(scanner);
    }

    // Create token
    Token* token = malloc(sizeof(Token));
    if (!token) return NULL;
    
    token->raw_text = NULL;

    // get full words for identifiers or other
    if (isalpha(scanner->current_char) || scanner->current_char == '_') {
        // set up a buffer so the string persists while we build it
        char buffer[256] = {0};
        int i = 0;

        // accepts alphanumeric so numbers are fine as long as it's not first
        while ((isalnum(scanner->current_char) || scanner->current_char == '_') && i < 255) {
            buffer[i++] = scanner->current_char;
            advance_char(scanner);
        }

        buffer[i] = '\0';

        TokenType keyword_type = IDENT; // Default to identifier

        // check all the other potential identifiers
        if (strcmp(buffer, "func") == 0) {
            keyword_type = FUNC;
        } else if (strcmp(buffer, "return") == 0) {
            keyword_type = RETURN;
        } else if (strcmp(buffer, "main") == 0) {
            keyword_type = MAIN;
        } else if (strcmp(buffer, "int") == 0) {
            keyword_type = INT;
        } else if (strcmp(buffer, "float") == 0) {
            keyword_type = FLOAT;
        } else if (strcmp(buffer, "char") == 0) {
            keyword_type = CHAR;
        } else if (strcmp(buffer, "string") == 0) {
            keyword_type = STRING;
        } else if (strcmp(buffer, "->") == 0) {
            keyword_type = RETURN_VALUE;
        } else if (strcmp(buffer, "bool") == 0) {
            keyword_type = BOOL;
        } else if (strcmp(buffer, "true") == 0) { // bool literals true
            token->type = BOOL_LITERAL;
            token->value.bool_value = true;
        } else if (strcmp(buffer, "false") == 0) { // bool literals false
            token->type = BOOL_LITERAL;
            token->value.bool_value = false;
        }

        token->type = keyword_type;
        token->raw_text = my_strdup(buffer);
        return token;

    }

    // for int and float literals
    if (isdigit(scanner->current_char)) {
        char buffer[256] = {0};
        int i = 0;
        bool has_decimal = false;
        
        // Read all consecutive digits and at most one decimal point
        while ((isdigit(scanner->current_char) || scanner->current_char == '.') && i < 255) {
            if (scanner->current_char == '.') {
                // If we already saw a decimal point, this is invalid
                if (has_decimal) {
                    break;  // Stop processing - this will be handled as a separate token
                }
                has_decimal = true;
            }
            
            buffer[i++] = scanner->current_char;
            advance_char(scanner);
        }
        buffer[i] = '\0';
        
        // Set the token type based on whether we found a decimal point
        token->type = has_decimal ? FLOAT_LITERAL : INT_LITERAL;
        token->raw_text = my_strdup(buffer);
        
        // Store the appropriate value based on the type
        if (has_decimal) {
            token->value.float_value = atof(buffer);
        } else {
            token->value.int_value = atoi(buffer);
        }
        
        return token;
    }

    // for string literals
    if (scanner->current_char == '"') {
        char buffer[256] = {};
        int i = 0;

        // skip initial quotes
        advance_char(scanner);

        while (scanner->current_char != '"') {
            buffer[i++] = scanner->current_char;
            advance_char(scanner);
        }
        buffer[i] = '\0';

        // skip final quotes
        advance_char(scanner);

        token->type = STRING_LITERAL;
        token->raw_text = my_strdup(buffer);
        token->value.string_value = my_strdup(buffer);
        return token;
    }

    // For char literals
    if (scanner->current_char == '\'') {
        // Skip initial quote
        advance_char(scanner);
        
        // Get the character
        char ch = scanner->current_char;
        advance_char(scanner);
        
        // Skip closing quote if it exists
        if (scanner->current_char == '\'') {
            advance_char(scanner);
        }
        
        // Create token
        token->type = CHAR_LITERAL;
        
        // Create a string representation for debugging
        char representation[4] = {'\'', ch, '\'', '\0'};
        token->raw_text = my_strdup(representation);
        
        // Store the character value
        token->value.int_value = (int)ch;
        
        return token;
    }

    // eat comment line, but not line end or file end
    if (scanner->current_char == '#') {
        token->type = COMMENT;
        token->raw_text = my_strdup("#");
        
        while(scanner->current_char != '\n' && scanner->current_char != '\1') {
            advance_char(scanner);
        }

        return token;
    }

    // Check for end of line
    if (scanner->current_char == '\n') {
        token->type = END_OF_LINE;
        token->raw_text = my_strdup("\\n");
        advance_char(scanner);
        return token;
    }
    
    // Check for end of file
    if (scanner->current_char == '\1') {
        token->type = END_OF_FILE;
        token->raw_text = my_strdup("EOF");
        return token;
    }

    // Handle single character tokens (operators, braces, etc)
    switch (scanner->current_char) {
        case '{':
            token->type = LBRACE;
            token->raw_text = my_strdup("{");
            break;
        case '}':
            token->type = RBRACE;
            token->raw_text = my_strdup("}");
            break;
        case '(':
            token->type = LPAREN;
            token->raw_text = my_strdup("(");
            break;
        case ')':
            token->type = RPAREN;
            token->raw_text = my_strdup(")");
            break;
        case '[':
            token->type = LBRACKET;
            token->raw_text = my_strdup("[");
            break;
        case ']':
            token->type = RBRACKET;
            token->raw_text = my_strdup("]");
            break;
        case '+':
            token->type = ADD;
            token->raw_text = my_strdup("+");
            break;
        case '-':
            token->type = SUBTRACT;
            token->raw_text = my_strdup("-");
            break;
        case '*':
            token->type = MULTIPLY;
            token->raw_text = my_strdup("*");
            break;
        case '/':
            token->type = QUOTIENT;
            token->raw_text = my_strdup("/");
            break;
        case '=':
            token->type = ASSIGN;
            token->raw_text = my_strdup("=");
            break;
        // Add other characters as needed
        default:
            // If no match, it's an illegal token
            token->type = ILLEGAL;
            char raw_text[2] = {scanner->current_char, '\0'};
            token->raw_text = my_strdup(raw_text);
            break;
    }

    // Don't forget to advance the scanner after processing
    advance_char(scanner);
    return token;
}

Token* getNextToken(Scanner* scanner) {
    Token* next_token = tokenize(scanner);

    // check capacity, add space just in case
    if (scanner->count == scanner->capacity) {
        // double capacity
        int new_capacity = scanner->capacity * 2;
        Token** new_tokens = realloc(scanner->tokens, sizeof(Token*) * new_capacity);

        scanner->capacity = new_capacity;
        scanner->tokens = new_tokens;
    }

    // if we're at this point we have plenty of size, can put next token in.
    scanner->tokens[scanner->count] = next_token;
    scanner->count++;
    scanner->position++;


    return next_token;
}

void goBackToken(Scanner* scanner) {
    // Get the token we want to remove
    Token* token = scanner->tokens[scanner->count - 1];
        
    // Go back in the file by the token's text length
    int token_length = strlen(token->raw_text) + 1;
    
    // Handle special cases for newlines and other tokens
    if (token->type == END_OF_LINE) {
        scanner->line--; // Go back one line
        // Need to find the column position at the end of the previous line
        // This is more complex and might require tracking
    } else {
        // For normal tokens, just go back by the token's length
        scanner->column -= token_length;
        fseek(scanner->file, -token_length, SEEK_CUR);
    }
    
    // Free the token and adjust the counters
    free_token(token);
    scanner->tokens[scanner->count - 1] = NULL; // Clear the pointer
    scanner->count--;
    scanner->position--;
    
}

// free the scanner and file we're scanning
void free_scanner(Scanner* scanner) {
    if (scanner) {
        // fclose because we store source file in scanner
        if (scanner->file) fclose(scanner->file);

        for (int i = 0; i < scanner->count; i++) {
            free_token(scanner->tokens[i]);
        }   
        free(scanner->tokens);

        free(scanner);
    }
}

