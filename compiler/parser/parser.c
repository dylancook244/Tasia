#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "parser.h"
#include "../scanner/scanner.h"
#include "../ast/ast.h"
#include "../symbol_table/symbol_table.h"

static char* my_strdup(const char* str) {
    size_t len = strlen(str) + 1;
    char* new_str = malloc(len);
    if (new_str) {
        memcpy(new_str, str, len);
    }
    return new_str;
}

AstNode* parse_program(Scanner* scanner, SymbolTable* symbol_table) {
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
            token->raw_text);
    
        // skip newlines
        if (token->type == END_OF_LINE) {
            token = getNextToken(scanner);
            continue;
        }
    
        // parse based on token type for top-level statements
        AstNode* node = NULL;
        switch (token->type) { 
            case FUNC:
                node = parse_function(scanner, token, symbol_table);
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
                continue;
            case COMMENT:
                // Just skip comments at the top level
                token = getNextToken(scanner);
                continue;
            // other cases... add later
            default: 
                printf("Unexpected top level statement token: %s", token_type_to_string(token->type));
                token = getNextToken(scanner);
                continue;
        }
        
        // Get next token
        token = getNextToken(scanner);
    }

    return (AstNode*)program;
}

void print_token_array(Scanner* scanner) {
    printf("\n=== TOKEN ARRAY RAW TEXT ===\n");
    printf("Count: %d, Position: %d\n", scanner->count, scanner->position);
    
    for (int i = 0; i < scanner->count; i++) {
        Token* token = scanner->tokens[i];
        if (token == NULL) {
            printf("[%d]: NULL\n", i);
        } else {
            printf("[%d]: '%s'%s\n", 
                   i,
                   token->raw_text,
                   (i == scanner->position - 1) ? " <- CURRENT" : "");
        }
    }
    printf("=== END TOKEN ARRAY ===\n");
}

// this function is really long, 
// partially due to a shit ton of crucial error messages
AstNode* parse_function(Scanner* scanner, Token* func_token, SymbolTable* symbol_table) {
    
    // Get function name
    Token* name_token = getNextToken(scanner);

    if (name_token->type != IDENT && name_token->type != MAIN) {
        printf("Error: Expected function name, got %s\n", token_type_to_string(name_token->type));
        return NULL;
    }
    
    // Save the function name
    // Check if this is the main function - if so, replace it with TASIA_ENTRY_FUNCTION
    char* func_name;
    if (name_token->type == MAIN || strcmp(name_token->raw_text, "main") == 0) {
        func_name = my_strdup("TASIA_ENTRY_FUNCTION");
        printf("\nRenaming 'main' function to 'TASIA_ENTRY_FUNCTION'\n");
    } else {
        // Otherwise use the name as-is
        func_name = my_strdup(name_token->raw_text);
    }
    
    
    // Check for left parenthesis
    Token* lparen = getNextToken(scanner);

    // Skip any newlines before the opening brace
    while (lparen->type == END_OF_LINE) {
        lparen = getNextToken(scanner);
    }

    if (lparen->type != LPAREN) {
        printf("Error: Expected '(' after function name, got %s\n", token_type_to_string(lparen->type));
        free(func_name);
        return NULL;
    }
    
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
                token = getNextToken(scanner);
            }
            
            // First token should be a type
            if (token->type != INT && token->type != FLOAT && 
                token->type != CHAR && token->type != STRING && 
                token->type != BOOL) {
                printf("Error: Expected parameter type, got %s\n", token_type_to_string(token->type));
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
            
            // Next token should be parameter name
            token = getNextToken(scanner);
            if (token->type != IDENT) {
                printf("Error: Expected parameter name, got %s\n", token_type_to_string(token->type));
                free(param_type);
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
    
    // Check for return type (->)
    char* return_type = NULL;
    token = getNextToken(scanner);
    if (token->type == RETURN_VALUE) {
        
        // Get the return type
        token = getNextToken(scanner);
        if (token->type != INT && token->type != FLOAT && 
            token->type != CHAR && token->type != STRING && 
            token->type != BOOL) {
            printf("Error: Expected return type, got %s\n", token_type_to_string(token->type));
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
        
        // Get next token
        token = getNextToken(scanner);
    }

    // even if the function doesn't have a return we still already got the next token
    if (token->type != LBRACE) {
        printf("Error: Expected '{' for function body, got %s\n", 
            token_type_to_string(token->type));
        return NULL;
    }

    // make sure function isn't already in the symbol table
    if (lookup_symbol_current_scope(symbol_table, func_name)) {
        printf("Error: what the hell fucking conveyed you to write the same damn function twice? TWICE!? '%s' is already defined idiot\n", func_name);
        // free parameters (there's a lot)
        for(int i = 0; i < param_count; i++) {
            free(param_names[i]);
            free(param_types[i]);
        }
        free(param_names);
        free(param_types);
        free(func_name);
        return NULL;
    }

    // add function to the symbol table
    Symbol* func_sym = add_symbol(
        symbol_table,
        func_name,
        SYMBOL_FUNCTION,
        return_type,
        param_types,
        param_count
    );

    if (!func_sym) {
        printf("Error: failed to add function '%s' to symbol_table\n", func_name);
        // free parameters (there's a lot)
        for(int i = 0; i < param_count; i++) {
            free(param_names[i]);
            free(param_types[i]);
        }
        free(param_names);
        free(param_types);
        free(func_name);
        return NULL;
    }
    
    // Create the function node
    FuncDeclNode* func = (FuncDeclNode*)create_func_node(func_name, return_type);
    func->param_names = param_names;
    func->param_types = param_types;
    func->param_count = param_count;

    // enter a new scope for function body
    enter_scope(symbol_table);

    // Add parameters to the symbol table
    for (int i = 0; i < param_count; i++) {
        Symbol* param_sym = add_symbol(
            symbol_table,
            param_names[i],
            SYMBOL_VARIABLE,
            param_types[i],
            NULL,
            0
        );
        
        if (param_sym) {
            // Parameters are initialized by default
            param_sym->is_initialized = true;
            // Parameters are immutable by default (can change this if you want)
            param_sym->is_mutable = false;
        }
    }

    // Parse the function body as a block
    func->body = parse_block(scanner, symbol_table);

    // Check if block parsing failed
    if (!func->body) {
        // now we're freeing the scope because freeing the ast node isn't enough
        exit_scope(symbol_table);
        free_ast_node((AstNode*)func);
        return NULL;
    }

    exit_scope(symbol_table);

    return (AstNode*)func;
}

AstNode* parse_block(Scanner* scanner, SymbolTable* symbol_table) {
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

    // create new scope for block
    enter_scope(symbol_table);

    // already ate opening brace, parse til closing brace
    Token* token = getNextToken(scanner);

    while (token->type != RBRACE && token->type != END_OF_FILE) {

        // skip newlines
        if (token->type == END_OF_LINE) {
            token = getNextToken(scanner);
            continue;
        }

        // parse next statement in the function
        AstNode* stmt = parse_statement(scanner, token, symbol_table);

        // add statement to block if valid
        if (stmt) {
            // resize block array of stmts if needed
            if (block->statement_count >= capacity) {
                capacity *= 2;
                block->statements = realloc(block->statements, sizeof(AstNode*) * capacity);
                
                if (!block->statements) {
                    // handle allocation failure
                    free_ast_node((AstNode*)block);
                    exit_scope(symbol_table);
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
        free_ast_node((AstNode*)block);
        exit_scope(symbol_table);
        return NULL;
    }

    exit_scope(symbol_table);
        
    return (AstNode*)block;
}

AstNode* parse_statement(Scanner* scanner, Token* token, SymbolTable* symbol_table) {
    // Decide what kind of statement to parse based on the token
    switch (token->type) {
        case INT:
        case FLOAT:
        case CHAR:
        case STRING:
        case BOOL:
            // This is a variable declaration
            return parse_var_decl(scanner, token, symbol_table);

        case INT_LITERAL:
        case FLOAT_LITERAL:
        case STRING_LITERAL:
        case CHAR_LITERAL:
        case BOOL_LITERAL:
            printf("Warning: Standalone literal has no effect\n");
            return parse_expr_stmt(scanner, token, symbol_table);
            
        case RETURN:
            return parse_return_stmt(scanner, symbol_table);
            
        case LBRACE:
            return parse_block(scanner, symbol_table);
            
        case IDENT:
            return parse_expr_stmt(scanner, token, symbol_table);
            
        default:
            printf("Error: Unexpected token in statement: %s\n", 
                   token_type_to_string(token->type));
            return NULL;
    }
}

AstNode* parse_var_decl(Scanner* scanner, Token* type_token, SymbolTable* symbol_table) {
    // Save the type
    char* type = my_strdup(type_token->raw_text);
    
    // Get the variable name (should be an identifier)
    Token* name_token = getNextToken(scanner);
    if (name_token->type != IDENT) {
        printf("Error: Expected variable name after type, got %s\n", 
              token_type_to_string(name_token->type));
        free(type);
        return NULL;
    }
    
    // Save the name
    char* name = my_strdup(name_token->raw_text);

    // Check if the variable already exists in the current scope
    if (lookup_symbol_current_scope(symbol_table, name)) {
        printf("Error: Variable '%s' already declared in this scope\n", name);
        free(type);
        free(name);
        return NULL;
    }

    // Check for assignment operator
    Token* assign_token = getNextToken(scanner);
    AstNode* initializer = NULL;
    bool is_initialized = false;
    
    if (assign_token->type == ASSIGN) {
        
        // Parse the initializer expression
        is_initialized = true;
        initializer = parse_expr(scanner, 0, symbol_table);
        
        if (!initializer) {
            printf("Error: Invalid initializer expression\n");
            free(type);
            free(name);
            return NULL;
        }
    }

    // Add variable to symbol table
    Symbol* var_symbol = add_symbol(
        symbol_table,
        name,
        SYMBOL_VARIABLE,
        type,
        NULL,
        0
    );
    
    if (!var_symbol) {
        printf("Error: Failed to add variable '%s' to symbol table\n", name);
        free(type);
        free(name);
        if (initializer) free_ast_node(initializer);
        return NULL;
    }
    
    // Set variable properties
    var_symbol->is_initialized = is_initialized;
    var_symbol->is_mutable = true;  // Default to mutable for now
    
    // Create the variable declaration node
    return create_var_node(name, type, initializer);
}

AstNode* parse_return_stmt(Scanner* scanner, SymbolTable* symbol_table) {
    // Create the return statement node
    ReturnStmtNode* ret_stmt = (ReturnStmtNode*)create_return_node();
    
    // Check if there's a return value
    Token* token = getNextToken(scanner);
    
    // Skip newlines in case there are any between return and the value
    while (token->type == END_OF_LINE) {
        token = getNextToken(scanner);
    }
    
    return (AstNode*)ret_stmt;
}

AstNode* parse_expr_stmt(Scanner* scanner, Token* token, SymbolTable* symbol_table) {
    goBackToken(scanner);
    AstNode* expr = parse_expr(scanner, 0, symbol_table);
    
    if (!expr) {
        return NULL;  // Error already printed in parse_expr
    }
    
    // Create an expression statement node
    ExprStmtNode* stmt = (ExprStmtNode*)create_expr_node(expr);
    
    // Check for statement terminator (line end, semicolon, etc.)
    Token* term = getNextToken(scanner);
    goBackToken(scanner);  // Put it back immediately
    
    if (term->type != END_OF_LINE && term->type != RBRACE) {
        printf("Warning: Expected end of statement, got %s\n", 
              token_type_to_string(term->type));
    }

    // Put back the terminator token if it's a closing brace
    if (term->type == RBRACE) {
        goBackToken(scanner);
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

// Helper to check if a token type is a binary operator
bool is_binary_operator(TokenType type) {
    return type == ADD || type == SUBTRACT || 
           type == MULTIPLY || type == QUOTIENT ||
           type == ASSIGN;
}

AstNode* parse_expr(Scanner* scanner, int min_bp, SymbolTable* symbol_table) {
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

            // Look up the identifier in the symbol table
            Symbol* symbol = lookup_symbol(symbol_table, name);
            if (!symbol) {
                printf("Error: Undeclared identifier '%s'\n", name);
                free(name);
                return NULL;
            }
            
            // Check if it's a function call
            token = getNextToken(scanner);
            if (token->type == LPAREN) {
                left = parse_call_args(scanner, name, symbol_table);
            } else {
                goBackToken(scanner);
                left = create_ident_node(name);
            }
            break;
        }
        
        case LPAREN:
            left = parse_expr(scanner, 0, symbol_table);
            token = getNextToken(scanner);
            if (token->type != RPAREN) {
                printf("Error: Expected closing parenthesis, got %s\n", 
                       token_type_to_string(token->type));
                free_ast_node(left);
                return NULL;
            }
            break;
            
        default:
            printf("Error: Unexpected token in expression: %s\n", 
                   token_type_to_string(token->type));
            return NULL;
    }
    
    // Now handle infix expressions (binary operations)
    while (1) {
        token = getNextToken(scanner);
        
        // If token is not an operator or binding power is too low, break
        if (!is_binary_operator(token->type)) {
            goBackToken(scanner);
            break;
        }
        
        Precedence p = get_precedence(token->type);
        if (p.left_bp < min_bp) {
            goBackToken(scanner);
            break;
        }
        
        TokenType op = token->type;
        
        AstNode* right = parse_expr(scanner, p.right_bp, symbol_table);
        if (!right) {
            free_ast_node(left);
            return NULL;
        }
        
        left = create_binexpr_node(left, op, right);

        // Mark variables as initialized after assignment
        if (op == ASSIGN && left->type == NODE_BINARY_EXPR) {
            BinaryExprNode* bin = (BinaryExprNode*)left;
            if (bin->left->type == NODE_IDENT_EXPR) {
                IdentExprNode* ident = (IdentExprNode*)bin->left;
                Symbol* symbol = lookup_symbol(symbol_table, ident->name);
                if (symbol) {
                    symbol->is_initialized = true;
                }
            }
        }
    }
    
    return left;
}

// Helper for parsing function call arguments
AstNode* parse_call_args(Scanner* scanner, char* func_name, SymbolTable* symbol_table) {
    // Create an identifier node for the function name
    AstNode* func_ident = create_ident_node(func_name);

    // Look up the function in the symbol table to get parameter info
    Symbol* func_sym = lookup_symbol(symbol_table, func_name);
    int expected_param_count = 0;
    if (func_sym && func_sym->kind == SYMBOL_FUNCTION) {
        expected_param_count = func_sym->param_count;
    }
    
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
        return (AstNode*)call;
    }
    
    // Parse arguments
    do {
        // If we've already processed some arguments,
        // token is a comma, so get the next token
        if (call->argument_count > 0) {
            token = getNextToken(scanner);
        }
        
        // Parse the argument expression
        goBackToken(scanner);
        AstNode* arg = parse_expr(scanner, 0, symbol_table);
        
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
        free_ast_node((AstNode*)call);
        return NULL;
    }
    
    return (AstNode*)call;
}

bool is_statement_start(TokenType type) {
    return type == INT || type == FLOAT || type == CHAR ||
           type == STRING || type == BOOL || type == IDENT ||
           type == RETURN || type == LBRACE;
           // Add more statement types as needed
}

