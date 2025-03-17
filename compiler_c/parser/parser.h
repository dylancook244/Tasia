#ifndef PARSER_H
#define PARSER_H

#include "../scanner/scanner.h"
#include "../ast/ast.h"

// Top-level parsing functions
AstNode* parse_program(Scanner* scanner);
AstNode* parse_func_decl(Scanner* scanner);

// Statement parsing
AstNode* parse_block(Scanner* scanner);
AstNode* parse_var_decl(Scanner* scanner);
AstNode* parse_expr_stmt(Scanner* scanner);
AstNode* parse_return_stmt(Scanner* scanner);

// Expression parsing
AstNode* parse_assign_expr(Scanner* scanner);
AstNode* parse_binary_expr(Scanner* scanner, AstNode* left);
AstNode* parse_ident_expr(Scanner* scanner);
AstNode* parse_literal_expr(Scanner* scanner);
AstNode* parse_call_expr(Scanner* scanner);

// Helper functions
AstNode* parse_expression(Scanner* scanner);
AstNode* parse_statement(Scanner* scanner);

#endif