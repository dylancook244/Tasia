#ifndef PARSER_H
#define PARSER_H

#include "../scanner/scanner.h"
#include "../ast/ast.h"

// Top-level parsing functions
AstNode* parse_program(Scanner* scanner);
AstNode* parse_function(Scanner* scanner, Token* func_token);

// Statement parsing
AstNode* parse_block(Scanner* scanner);
AstNode* parse_var_decl(Scanner* scanner, Token* type_token);
AstNode* parse_expr_stmt(Scanner* scanner, Token* token);
AstNode* parse_return_stmt(Scanner* scanner);

// Expression parsing
AstNode* parse_assign_expr(Scanner* scanner);
AstNode* parse_binary_expr(Scanner* scanner, AstNode* left, int min_precedence);AstNode* parse_ident_expr(Scanner* scanner, Token* token);
AstNode* parse_literal_expr(Scanner* scanner, Token* token);
AstNode* parse_call_expr(Scanner* scanner, AstNode* function);

// Helper functions
AstNode* parse_expression(Scanner* scanner, Token* token);
AstNode* parse_statement(Scanner* scanner, Token* token);

AstNode* parse_primary_expr(Scanner* scanner, Token* token);
bool is_binary_operator(TokenType type);
int get_operator_precedence(TokenType op);

void ungetToken(Scanner* scanner, Token* token);

#endif