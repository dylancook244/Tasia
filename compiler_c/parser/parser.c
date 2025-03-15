#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "parser.h"
#include "../scanner/scanner.h"
#include "../ast/ast.h"

AstNode* parse_function(Scanner* scanner, Token* func_token);

static char* my_strdup(const char* str) {
    size_t len = strlen(str) + 1;
    char* new_str = malloc(len);
    if (new_str) {
        memcpy(new_str, str, len);
    }
    return new_str;
}

AstNode* parse_program(Scanner* scanner) {
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


    // Actual parsing
    ProgramNode* program = (ProgramNode*)create_program_node();
    program->statements = malloc(sizeof(AstNode*) * 10);
    int capacity = 10;

    Token* token = getNextToken(scanner);
    
    while (token->type != END_OF_FILE) {
        // Process token
        printf("\nToken: %s, raw_text: '%s', Line: %d, Col: %d",
            token_type_to_string(token->type),
            token->raw_text,
            token->line,
            token->column);
    
        // skip newlines
        if (token->type == END_OF_LINE) {
            free_token(token);
            token = getNextToken(scanner);
            continue;
        }
    
        // parse based on token type for top-level statements
        AstNode* node = NULL;
        switch (token->type) { 
            case FUNC:
                node = parse_function(scanner, token);
                // Add the node to the program's statements array
                if (node != NULL) {
                    if (program->statement_count >= capacity) {
                        capacity *= 2;
                        program->statements = realloc(program->statements, sizeof(AstNode*) * capacity);
                    }
                    program->statements[program->statement_count++] = node;
                }
                // Don't free the token here - it's already freed in parse_function
                token = getNextToken(scanner);
                continue;  // Skip the free_token below
            case COMMENT:
                // Just skip comments at the top level
                free_token(token);
                token = getNextToken(scanner);
                continue;
            // other cases... add later
            default: 
                printf("Unexpected top level statement token: %s", token_type_to_string(token->type));
                free_token(token);
                token = getNextToken(scanner);
                continue;
        }
     
        // Free the token - only tokens that fall through to here
        free_token(token);
        
        // Get next token
        token = getNextToken(scanner);
    }

    // free last EOF token
    free_token(token);
    return (AstNode*)program;
}

// this function is really long, 
// partially due to a shit ton of crucial error messages
AstNode* parse_function(Scanner* scanner, Token* func_token) {
    // We already know it's a FUNC token
    free_token(func_token);
    
    // Get function name
    Token* name_token = getNextToken(scanner);

    if (name_token->type != IDENT && name_token->type != MAIN) {
        printf("Error: Expected function name, got %s\n", token_type_to_string(name_token->type));
        free_token(name_token);
        return NULL;
    }
    
    // Save the function name
    char* func_name = my_strdup(name_token->raw_text);
    free_token(name_token);
    
    // Check for left parenthesis
    Token* lparen = getNextToken(scanner);
    if (lparen->type != LPAREN) {
        printf("Error: Expected '(' after function name, got %s\n", token_type_to_string(lparen->type));
        free(func_name);
        free_token(lparen);
        return NULL;
    }
    free_token(lparen);
    
    // Parse parameters
    int param_count = 0;
    char** param_names = NULL;
    char** param_types = NULL;
    
    // Check if there are any parameters
    Token* token = getNextToken(scanner);
    if (token->type != RPAREN) {
        // We have at least one parameter
        do {
            // If we've already read some parameters, we expect a type token
            // after a comma, so get the next token
            if (param_count > 0) {
                free_token(token); // Free the comma
                token = getNextToken(scanner);
            }
            
            // First token should be a type
            if (token->type != INT && token->type != FLOAT && 
                token->type != CHAR && token->type != STRING && 
                token->type != BOOL) {
                printf("Error: Expected parameter type, got %s\n", token_type_to_string(token->type));
                free_token(token);
                // Free existing parameters
                for (int i = 0; i < param_count; i++) {
                    free(param_names[i]);
                    free(param_types[i]);
                }
                free(param_names);
                free(param_types);
                free(func_name);
                return NULL;
            }
            
            // Save parameter type
            char* param_type = my_strdup(token->raw_text);
            free_token(token);
            
            // Next token should be parameter name
            token = getNextToken(scanner);
            if (token->type != IDENT) {
                printf("Error: Expected parameter name, got %s\n", token_type_to_string(token->type));
                free(param_type);
                free_token(token);
                // Free existing parameters
                for (int i = 0; i < param_count; i++) {
                    free(param_names[i]);
                    free(param_types[i]);
                }
                free(param_names);
                free(param_types);
                free(func_name);
                return NULL;
            }
            
            // Save parameter name
            char* param_name = my_strdup(token->raw_text);
            free_token(token);
            
            // Add to parameter arrays
            param_count++;
            param_names = realloc(param_names, param_count * sizeof(char*));
            param_types = realloc(param_types, param_count * sizeof(char*));
            param_names[param_count-1] = param_name;
            param_types[param_count-1] = param_type;
            
            // Check for comma or closing paren
            token = getNextToken(scanner);
        } while (token->type == COMMA);
        
        // After parameters, we expect a right parenthesis
        if (token->type != RPAREN) {
            printf("Error: Expected ')' after parameters, got %s\n", token_type_to_string(token->type));
            free_token(token);
            // Free parameters
            for (int i = 0; i < param_count; i++) {
                free(param_names[i]);
                free(param_types[i]);
            }
            free(param_names);
            free(param_types);
            free(func_name);
            return NULL;
        }
    }
    
    // Free the right parenthesis token
    free_token(token);
    
    // Check for return type (->)
    char* return_type = NULL;
    token = getNextToken(scanner);
    if (token->type == RETURN_VALUE) {
        free_token(token);
        
        // Get the return type
        token = getNextToken(scanner);
        if (token->type != INT && token->type != FLOAT && 
            token->type != CHAR && token->type != STRING && 
            token->type != BOOL) {
            printf("Error: Expected return type, got %s\n", token_type_to_string(token->type));
            free_token(token);
            // Free parameters
            for (int i = 0; i < param_count; i++) {
                free(param_names[i]);
                free(param_types[i]);
            }
            free(param_names);
            free(param_types);
            free(func_name);
            return NULL;
        }
        
        return_type = my_strdup(token->raw_text);
        free_token(token);
        
        // Get next token
        token = getNextToken(scanner);
    }
    
    // Create the function node
    FuncDeclNode* func = (FuncDeclNode*)create_func_node(func_name, return_type);
    func->param_names = param_names;
    func->param_types = param_types;
    func->param_count = param_count;
    
    // We should now have a left brace for the function body
    if (token->type != LBRACE) {
        printf("Error: Expected '{' for function body, got %s\n", token_type_to_string(token->type));
        free_token(token);
        free_ast_node((AstNode*)func);
        return NULL;
    }
    free_token(token);

    
    
    // For now, just skip everything until the matching right brace
    // In a real implementation, you would call parse_block here
    int brace_count = 1;
    while (brace_count > 0) {
        token = getNextToken(scanner);
        if (token->type == LBRACE) {
            brace_count++;
        } else if (token->type == RBRACE) {
            brace_count--;
        } else if (token->type == END_OF_FILE) {
            printf("Error: Unexpected end of file in function body\n");
            free_token(token);
            free_ast_node((AstNode*)func);
            return NULL;
        }
        free_token(token);
    }
    
    // Eventually you would parse the block and set func->body
    
    return (AstNode*)func;
}