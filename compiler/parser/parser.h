#ifndef PARSER_H
#define PARSER_H

#include "../scanner/scanner.h"
#include "../ast/ast.h"

// We're using Pratt parsing for this compiler.
// In Pratt parsing, operators have binding powers that determine precedence.
// Each operator has left_bp (how strongly it binds to its left operand)
// and right_bp (how strongly it binds to its right operand).
// Higher numbers = higher precedence. For 3 + 5 * 2, the * has higher
// binding power (10,11) than + (8,9), so * takes precedence. 

typedef struct {
    int left_bp; 
    int right_bp;
} Precedence;

// Top-level parsing functions
AstNode* parse_program(Scanner* scanner);
AstNode* parse_function(Scanner* scanner, Token* func_token);

// Statement parsing
AstNode* parse_block(Scanner* scanner);
AstNode* parse_var_decl(Scanner* scanner, Token* type_token);
AstNode* parse_expr_stmt(Scanner* scanner, Token* token);
AstNode* parse_return_stmt(Scanner* scanner);
AstNode* parse_statement(Scanner* scanner, Token* token);

// Expression parsing
AstNode* parse_expr(Scanner* scanner, int min_bp);
AstNode* parse_call_args(Scanner* scanner, char* func_name);

bool is_binary_operator(TokenType type);
Precedence get_precedence(TokenType type);
void ungetToken(Scanner* scanner, Token* token);

#endif