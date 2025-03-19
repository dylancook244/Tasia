#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "parser.h"
#include "../scanner/scanner.h"
#include "../ast/ast.h"

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
    // Re-initialize scanner's current_char, get first char as int
    int c = fgetc(scanner->file);
    // \1 is our EOF char
    scanner->current_char = (c == EOF) ? '\1' : (char)c;
    scanner->line = 1;
    scanner->column = 1;

    // Print the source code
    while (scanner->current_char != '\1') {  
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
    // Check if this is the main function - if so, replace it with TASIA_ENTRY_FUNCTION
    char* func_name;
    if (name_token->type == MAIN || strcmp(name_token->raw_text, "main") == 0) {
        func_name = my_strdup("TASIA_ENTRY_FUNCTION");
        printf("Renaming 'main' function to 'TASIA_ENTRY_FUNCTION'\n");
    } else {
        // Otherwise use the name as-is
        func_name = my_strdup(name_token->raw_text);
    }
    
    free_token(name_token);
    
    // Check for left parenthesis
    Token* lparen = getNextToken(scanner);

    // Skip any newlines before the opening brace
    while (lparen->type == END_OF_LINE) {
        free_token(lparen);
        lparen = getNextToken(scanner);
    }

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

    // even if the function doesn't have a return we still already got the next token
    if (token->type != LBRACE) {
        printf("Error: Expected '{' for function body, got %s\n", 
            token_type_to_string(token->type));
        free_token(token);
        // Free other resources...
        return NULL;
    }
    free_token(token);  // Free the opening brace token
    
    // Create the function node
    FuncDeclNode* func = (FuncDeclNode*)create_func_node(func_name, return_type);
    func->param_names = param_names;
    func->param_types = param_types;
    func->param_count = param_count;

    // Parse the function body as a block
    func->body = parse_block(scanner);

    // Check if block parsing failed
    if (!func->body) {
        free_ast_node((AstNode*)func);
        return NULL;
    }

    return (AstNode*)func;
}

AstNode* parse_block(Scanner* scanner) {
    // create a block node
    BlockNode* block = (BlockNode*)create_block_node();
    if (!block) return NULL;

    // statement array capacity
    int capacity = 10;

    block->statements = malloc(sizeof(AstNode*) * capacity);
    if (!block->statements) {
        free_ast_node((AstNode*)block);
        return NULL;
    }

    // already ate opening brace, parse til closing brace
    Token* token = getNextToken(scanner);

    while (token->type != RBRACE && token->type != END_OF_FILE) {

        // skip newlines
        if (token->type == END_OF_LINE) {
            free_token(token);
            token = getNextToken(scanner);
            continue;
        }

        // parse next statement in the function
        AstNode* stmt = parse_statement(scanner, token);

        // add statement to block if valid
        if (stmt) {
            // resize block array of stmts if needed
            if (block->statement_count >= capacity) {
                capacity *= 2;
                block->statements = realloc(block->statements, sizeof(AstNode*) * capacity);
                
                if (!block->statements) {
                    // handle allocation failure
                    free_ast_node((AstNode*)block);
                    return NULL;
                }
            }

            // add statement to block
            block->statements[block->statement_count++] = stmt;
        }

        // get next token for loop
        token = getNextToken(scanner);
    }

    // if end of file after function block
    if (token->type == END_OF_FILE) {
        printf("Error: Unexpected end of file, expected '}'\n");
        free_token(token);
        free_ast_node((AstNode*)block);
        return NULL;
    }
    
    // Free the closing brace token
    free_token(token);
    
    return (AstNode*)block;
}

AstNode* parse_statement(Scanner* scanner, Token* token) {
    // Decide what kind of statement to parse based on the token
    switch (token->type) {
        case INT:
        case FLOAT:
        case CHAR:
        case STRING:
        case BOOL:
            // This is a variable declaration
            return parse_var_decl(scanner, token);

        case INT_LITERAL:
        case FLOAT_LITERAL:
        case STRING_LITERAL:
        case CHAR_LITERAL:
        case BOOL_LITERAL:
            printf("Warning: Standalone literal has no effect\n");
            return parse_expr_stmt(scanner, token);
            
        case RETURN:
            // This is a return statement
            free_token(token);
            return parse_return_stmt(scanner);
            
        case LBRACE:
            // This is a nested block
            free_token(token);
            return parse_block(scanner);
            
        case IDENT:
            // This could be an assignment or a function call
            return parse_expr_stmt(scanner, token);
            
        default:
            printf("Error: Unexpected token in statement: %s\n", 
                   token_type_to_string(token->type));
            free_token(token);
            return NULL;
    }
}

AstNode* parse_var_decl(Scanner* scanner, Token* type_token) {
    // Save the type
    char* type = my_strdup(type_token->raw_text);
    free_token(type_token);
    
    // Get the variable name (should be an identifier)
    Token* name_token = getNextToken(scanner);
    if (name_token->type != IDENT) {
        printf("Error: Expected variable name after type, got %s\n", 
              token_type_to_string(name_token->type));
        free(type);
        free_token(name_token);
        return NULL;
    }
    
    // Save the name
    char* name = my_strdup(name_token->raw_text);
    free_token(name_token);
    
    // Check for assignment operator
    Token* assign_token = getNextToken(scanner);
    AstNode* initializer = NULL;
    
    if (assign_token->type == ASSIGN) {
        // There's an initializer
        free_token(assign_token);
        
        // Parse the initializer expression
        initializer = parse_expr(scanner, 0);
        
        if (!initializer) {
            printf("Error: Invalid initializer expression\n");
            free(type);
            free(name);
            return NULL;
        }
    } else {
        // No assignment, so we put back the token we just read
        free_token(assign_token);
    }
    
    // Create the variable declaration node
    return create_var_node(name, type, initializer);
}

AstNode* parse_return_stmt(Scanner* scanner) {
    // Create the return statement node
    ReturnStmtNode* ret_stmt = (ReturnStmtNode*)create_return_node();
    
    // Check if there's a return value
    Token* token = getNextToken(scanner);
    
    // Skip newlines in case there are any between return and the value
    while (token->type == END_OF_LINE) {
        free_token(token);
        token = getNextToken(scanner);
    }
    
    // If next token isn't a statement terminator, it's a return value
    if (token->type != END_OF_LINE && token->type != RBRACE) {
        ungetToken(scanner, token);  // Put the token back
        ret_stmt->value = parse_expr(scanner, 0);
        
        if (!ret_stmt->value) {
            printf("Error: Invalid return value expression\n");
            free_ast_node((AstNode*)ret_stmt);
            return NULL;
        }
    } else {
        // No return value, put back the token we just read
        ungetToken(scanner, token);  // Note: You'd need to implement this
    }
    
    return (AstNode*)ret_stmt;
}

AstNode* parse_expr_stmt(Scanner* scanner, Token* token) {
    // Parse the expression
    ungetToken(scanner, token);  // Put the token back
    AstNode* expr = parse_expr(scanner, 0);
    
    if (!expr) {
        return NULL;  // Error already printed in parse_expr
    }
    
    // Create an expression statement node
    ExprStmtNode* stmt = (ExprStmtNode*)create_expr_node(expr);
    
    // Check for statement terminator (line end, semicolon, etc.)
    Token* term = getNextToken(scanner);
    
    if (term->type != END_OF_LINE && term->type != RBRACE) {
        printf("Warning: Expected end of statement, got %s\n", 
              token_type_to_string(term->type));
    }
    
    // Put back the terminator token if it's a closing brace
    if (term->type == RBRACE) {
        ungetToken(scanner, term);  // Note: You'd need to implement this
    } else {
        free_token(term);
    }
    
    return (AstNode*)stmt;
}

// Helper function to get operator precedence
Precedence get_precedence(TokenType type) {
    switch (type) {
        case MULTIPLY: case QUOTIENT:
            return (Precedence){10, 11};
        case ADD: case SUBTRACT:
            return (Precedence){8, 9};
        case ASSIGN:
            return (Precedence){2, 1};
        default:
            return (Precedence){0, 0}; // not an operator
    }
}

void ungetToken(Scanner* scanner, Token* token) {
    if (scanner->has_buffered_token) {
        // If already have a buffered token, free it
        free_token(scanner->buffered_token);
    }
    
    scanner->buffered_token = token;
    scanner->has_buffered_token = true;
}

// Helper to check if a token type is a binary operator
bool is_binary_operator(TokenType type) {
    return type == ADD || type == SUBTRACT || 
           type == MULTIPLY || type == QUOTIENT ||
           type == ASSIGN;
}

AstNode* parse_expr(Scanner* scanner, int min_bp) {
    Token* token = getNextToken(scanner);
    
    AstNode* left;
    
    // Handle prefix expressions (literals, variables, etc.)
    switch(token->type) {
        case INT_LITERAL: case FLOAT_LITERAL: case CHAR_LITERAL:
        case STRING_LITERAL: case BOOL_LITERAL:
            left = create_literal_node(token);
            break;
            
        case IDENT: {
            char* name = my_strdup(token->raw_text);
            free_token(token);
            
            // Check if it's a function call
            token = getNextToken(scanner);
            if (token->type == LPAREN) {
                free_token(token);
                left = parse_call_args(scanner, name);
            } else {
                ungetToken(scanner, token);
                left = create_ident_node(name);
            }
            break;
        }
        
        case LPAREN:
            free_token(token);
            left = parse_expr(scanner, 0);
            token = getNextToken(scanner);
            if (token->type != RPAREN) {
                printf("Error: Expected closing parenthesis, got %s\n", 
                       token_type_to_string(token->type));
                free_token(token);
                free_ast_node(left);
                return NULL;
            }
            free_token(token);
            break;
            
        default:
            printf("Error: Unexpected token in expression: %s\n", 
                   token_type_to_string(token->type));
            free_token(token);
            return NULL;
    }
    
    // Now handle infix expressions (binary operations)
    while (1) {
        token = getNextToken(scanner);
        
        // If token is not an operator or binding power is too low, break
        if (!is_binary_operator(token->type)) {
            ungetToken(scanner, token);
            break;
        }
        
        Precedence p = get_precedence(token->type);
        if (p.left_bp < min_bp) {
            ungetToken(scanner, token);
            break;
        }
        
        TokenType op = token->type;
        free_token(token);
        
        AstNode* right = parse_expr(scanner, p.right_bp);
        if (!right) {
            free_ast_node(left);
            return NULL;
        }
        
        left = create_binexpr_node(left, op, right);
    }
    
    return left;
}

// Helper for parsing function call arguments
AstNode* parse_call_args(Scanner* scanner, char* func_name) {
    // Create an identifier node for the function name
    AstNode* func_ident = create_ident_node(func_name);
    
    // Create a call expression node
    CallExprNode* call = (CallExprNode*)create_call_node(func_ident);
    
    // Prepare to collect arguments
    int capacity = 4;
    call->arguments = malloc(sizeof(AstNode*) * capacity);
    
    if (!call->arguments) {
        printf("Error: Memory allocation failed\n");
        free_ast_node((AstNode*)call);
        return NULL;
    }
    
    // Check for empty argument list
    Token* token = getNextToken(scanner);
    if (token->type == RPAREN) {
        // No arguments
        free_token(token);
        return (AstNode*)call;
    }
    
    // Parse arguments
    do {
        // If we've already processed some arguments,
        // token is a comma, so get the next token
        if (call->argument_count > 0) {
            free_token(token);
            token = getNextToken(scanner);
        }
        
        // Parse the argument expression
        ungetToken(scanner, token);
        AstNode* arg = parse_expr(scanner, 0);
        
        if (!arg) {
            printf("Error: Invalid function argument\n");
            free_ast_node((AstNode*)call);
            return NULL;
        }
        
        // Add the argument to the call
        if (call->argument_count >= capacity) {
            capacity *= 2;
            call->arguments = realloc(call->arguments, sizeof(AstNode*) * capacity);
            
            if (!call->arguments) {
                printf("Error: Memory allocation failed\n");
                free_ast_node(arg);
                free_ast_node((AstNode*)call);
                return NULL;
            }
        }
        
        call->arguments[call->argument_count++] = arg;
        
        // Get next token (comma or closing parenthesis)
        token = getNextToken(scanner);
    } while (token->type == COMMA);
    
    // Expect closing parenthesis
    if (token->type != RPAREN) {
        printf("Error: Expected ')' or ',' in function call, got %s\n", 
               token_type_to_string(token->type));
        free_token(token);
        free_ast_node((AstNode*)call);
        return NULL;
    }
    
    free_token(token);
    return (AstNode*)call;
}

bool is_statement_start(TokenType type) {
    return type == INT || type == FLOAT || type == CHAR ||
           type == STRING || type == BOOL || type == IDENT ||
           type == RETURN || type == LBRACE;
           // Add more statement types as needed
}