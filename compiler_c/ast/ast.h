#ifndef AST_H
#define AST_H

#include <stdbool.h>
#include "../scanner/token.h"

// Base node type for all AST nodes
typedef enum {
    NODE_PROGRAM,       // Top level program
    NODE_FUNC_DECL,     // Function declaration
    NODE_BLOCK,         // Code block
    NODE_VAR_DECL,      // Variable declaration
    NODE_ARRAY_DECL,    // Array declaration
    NODE_LIST_DECL,     // List declaration
    NODE_TUPLE_DECL,    // Tuple declaration
    NODE_EXPR_STMT,     // Expression statement
    NODE_RETURN_STMT,   // Return statement
    NODE_ASSIGN_EXPR,   // Assignment expression
    NODE_BINARY_EXPR,   // Binary expression (math, comparison)
    NODE_IDENT_EXPR,    // Identifier reference
    NODE_LITERAL_EXPR,  // Literal value (int, float, bool, etc.)
    NODE_CALL_EXPR      // Function call
} NodeType;

// Forward declaration for generic node
typedef struct AstNode AstNode;

// Base struct for all AST nodes
struct AstNode {
    NodeType type;
    // Free function specific to this node type
    void (*free_func)(AstNode*);
};

// Program node - the root of the AST
typedef struct {
    AstNode base;              // Base node properties
    AstNode** statements;      // Array of statement nodes (mostly func nodes)
    int statement_count;       // Number of statements
} ProgramNode;

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

// Block node - represents a code block with multiple statements
typedef struct {
    AstNode base;
    AstNode** statements;
    int statement_count;
} BlockNode;

// Variable declaration node, nothing inferred at the moment
typedef struct {
    AstNode base;
    char* name;                // Variable name
    char* type;                // Variable type
    AstNode* initializer;      // Initial value expression
    bool is_mutable;           // Whether this variable can be modified
    bool is_reference;         // whether this is a reference to another value
} VarDeclNode;

// Array declaration node
typedef struct {
    AstNode base;
    struct {   // List of initializers for arrays, lists, tuples
        AstNode** elements;
        int count;
    } list;
    
} ArrayDeclNode;

// Expression statement node (expression followed by semicolon)
typedef struct {
    AstNode base;
    AstNode* expression;       // The expression
} ExprStmtNode;

// Return statement node
typedef struct {
    AstNode base;
    AstNode* value;            // Return value expression (may be NULL)
} ReturnStmtNode;

// Assignment expression node (a = b)
typedef struct {
    AstNode base;
    AstNode* target;           // Target to assign to (usually identifier)
    AstNode* value;            // Value to assign
} AssignExprNode;

// Binary expression node (a + b, a * b, etc.)
typedef struct {
    AstNode base;
    AstNode* left;             // Left operand
    TokenType operator;        // Operator (ADD, SUBTRACT, etc.)
    AstNode* right;            // Right operand
} BinaryExprNode;

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
        char char_value;
    } value;
} LiteralExprNode;

// Function call expression node
typedef struct {
    AstNode base;
    AstNode* function;         // Function to call (usually identifier)
    AstNode** arguments;       // Argument expressions
    int argument_count;        // Number of arguments
} CallExprNode;

// Free functions
void free_ast_node(AstNode* node);

// functions for creating nodes
AstNode* create_program_node();
AstNode* create_func_node(char* name, char* return_type);
AstNode* create_block_node();
AstNode* create_var_node(char* name, char* type, AstNode* initializer);
AstNode* create_array_node(int size, AstNode** elements);
AstNode* create_return_node();
AstNode* create_expr_node(AstNode* expression);
AstNode* create_assign_node();
AstNode* create_binexpr_node(AstNode* left, TokenType operator, AstNode* right);
AstNode* create_ident_node(char* name);
AstNode* create_literal_node(Token* token);
AstNode* create_call_node(AstNode* function);

void dump_ast(AstNode* node, int indent);

#endif