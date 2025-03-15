// ast.h
#ifndef AST_H
#define AST_H

#include <stdbool.h>
#include "scanner/token.h"

// Base node type for all AST nodes
typedef enum {
    NODE_PROGRAM,       // Top level program
    NODE_BLOCK,         // Code block
    NODE_FUNC_DECL,     // Function declaration
    NODE_VAR_DECL,      // Variable declaration
    NODE_EXPR_STMT,     // Expression statement
    NODE_RETURN_STMT,   // Return statement
    NODE_ASSIGN_EXPR,   // Assignment expression
    NODE_BINARY_EXPR,   // Binary expression (math, comparison)
    NODE_IDENT_EXPR,    // Identifier reference
    NODE_LITERAL_EXPR,  // Literal value (int, float, bool, etc.)
    NODE_CALL_EXPR      // Function call
    // Add more as needed
} NodeType;

// Forward declaration for generic node
typedef struct AstNode AstNode;

// Base struct for all AST nodes
struct AstNode {
    NodeType type;
    int line;
    int column;
    // Free function specific to this node type
    void (*free_func)(AstNode*);
};

// Program node - the root of the AST
typedef struct {
    AstNode base;              // Base node properties
    AstNode** statements;      // Array of statement nodes
    int statement_count;       // Number of statements
} ProgramNode;

// Block node - represents a code block with multiple statements
typedef struct {
    AstNode base;
    AstNode** statements;
    int statement_count;
} BlockNode;

// Function declaration node
typedef struct {
    AstNode base;
    char* name;                // Function name
    char** param_names;        // Parameter names
    char** param_types;        // Parameter types
    int param_count;           // Number of parameters
    char* return_type;         // Return type (NULL for void)
    AstNode* body;             // Function body (BlockNode)
} FuncDeclNode;

// Variable declaration node
typedef struct {
    AstNode base;
    char* name;                // Variable name
    char* type;                // Variable type (may be inferred)
    AstNode* initializer;      // Initial value expression (may be NULL)
    bool is_mutable;           // Whether this variable can be modified
} VarDeclNode;

// Return statement node
typedef struct {
    AstNode base;
    AstNode* value;            // Return value expression (may be NULL)
} ReturnStmtNode;

// Expression statement node (expression followed by semicolon)
typedef struct {
    AstNode base;
    AstNode* expression;       // The expression
} ExprStmtNode;

// Binary expression node (a + b, a * b, etc.)
typedef struct {
    AstNode base;
    AstNode* left;             // Left operand
    TokenType operator;        // Operator (ADD, SUBTRACT, etc.)
    AstNode* right;            // Right operand
} BinaryExprNode;

// Assignment expression node (a = b)
typedef struct {
    AstNode base;
    AstNode* target;           // Target to assign to (usually identifier)
    AstNode* value;            // Value to assign
} AssignExprNode;

// Identifier expression node (variable reference)
typedef struct {
    AstNode base;
    char* name;                // Identifier name
} IdentExprNode;

// Literal expression node (constants)
typedef struct {
    AstNode base;
    TokenType literal_type;    // INT_LITERAL, FLOAT_LITERAL, etc.
    union {
        int int_value;
        float float_value;
        bool bool_value;
        char* string_value;
    } value;
} LiteralExprNode;

// Function call expression node
typedef struct {
    AstNode base;
    AstNode* function;         // Function to call (usually identifier)
    AstNode** arguments;       // Argument expressions
    int argument_count;        // Number of arguments
} CallExprNode;

// AST creation functions
ProgramNode* create_program_node();
BlockNode* create_block_node();
FuncDeclNode* create_func_decl_node(char* name, char* return_type);
VarDeclNode* create_var_decl_node(char* name, char* type, bool is_mutable);
ReturnStmtNode* create_return_stmt_node(AstNode* value);
ExprStmtNode* create_expr_stmt_node(AstNode* expression);
BinaryExprNode* create_binary_expr_node(AstNode* left, TokenType operator, AstNode* right);
AssignExprNode* create_assign_expr_node(AstNode* target, AstNode* value);
IdentExprNode* create_ident_expr_node(char* name);
LiteralExprNode* create_literal_expr_node(TokenType literal_type);
CallExprNode* create_call_expr_node(AstNode* function);

// Free functions
void free_ast_node(AstNode* node);

#endif // AST_H