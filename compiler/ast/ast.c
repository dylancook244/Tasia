#include <stdlib.h>
#include <string.h> 
#include <stdio.h> 

#include "ast.h"

static char* my_strdup(const char* str) {
    size_t len = strlen(str) + 1;
    char* new_str = malloc(len);
    if (new_str) {
        memcpy(new_str, str, len);
    }
    return new_str;
}

// this should free all nodes, add here when making a new one
// recursively deletes the nodes as it passes through the ast
void free_ast_node(AstNode* node) {
    if (!node) return;
    
    switch (node->type) {
        case NODE_PROGRAM: {
            ProgramNode* program = (ProgramNode*)node;
            for (int i = 0; i < program->statement_count; i++) {
                free_ast_node(program->statements[i]); // mostly func nodes
            }
            free(program->statements);
            break;
        }
        case NODE_FUNC_DECL: {
            FuncDeclNode* func = (FuncDeclNode*)node;
            free(func->name);
            if (func->return_type) free(func->return_type);
            // Free params
            for (int i = 0; i < func->param_count; i++) {
                free(func->param_names[i]);
                free(func->param_types[i]);
            }
            free(func->param_names);
            free(func->param_types);
            if (func->body) free_ast_node(func->body);
            break;
        }
        case NODE_BLOCK: {
            BlockNode* block = (BlockNode*)node;
            for (int i = 0; i < block->statement_count; i++) {
                free_ast_node(block->statements[i]); // mostly expr nodes
            }
            free(block->statements);
            break;
        }
        case NODE_VAR_DECL: {
            VarDeclNode* var = (VarDeclNode*)node;
            free(var->name);
            free(var->type);
            if (var->initializer) free_ast_node(var->initializer);
            break;
        }
        case NODE_EXPR_STMT: {
            ExprStmtNode* expr = (ExprStmtNode*)node;
            if (expr->expression) free_ast_node(expr->expression);
            break;
        }
        case NODE_RETURN_STMT: {
            ReturnStmtNode* ret_stmt = (ReturnStmtNode*)node;
            if (ret_stmt->value) free_ast_node(ret_stmt->value);
            break;
        }
        case NODE_ASSIGN_EXPR: {
            AssignExprNode* assign = (AssignExprNode*)node;
            if (assign->target) free_ast_node(assign->target);
            if (assign->value) free_ast_node(assign->value);
            break;
        }
        case NODE_BINARY_EXPR: {
            BinaryExprNode* binexpr = (BinaryExprNode*)node;
            if (binexpr->left) free_ast_node(binexpr->left);
            if (binexpr->right) free_ast_node(binexpr->right);
            break;
        }   
        case NODE_IDENT_EXPR: {
            IdentExprNode* ident = (IdentExprNode*)node;
            free(ident->name);
            break;
        }
        case NODE_LITERAL_EXPR: {
            LiteralExprNode* literal = (LiteralExprNode*)node;
            if (literal->literal_type == STRING_LITERAL && literal->value.string_value) {
                free(literal->value.string_value);
            }
            break;
        }
        case NODE_CALL_EXPR: {
            CallExprNode* call = (CallExprNode*)node;
            if (call->function) free_ast_node(call->function);
            for (int i = 0; i < call->argument_count; i++) {
                free_ast_node(call->arguments[i]);
            }
            free(call->arguments);
            break;
        }
    }
    
    free(node);
}

AstNode* create_program_node() {
    ProgramNode* program = malloc(sizeof(ProgramNode));
    if (!program) return NULL;
    
    program->base.type = NODE_PROGRAM;
    program->statements = NULL;
    program->statement_count = 0;
    
    return (AstNode*)program;
}

AstNode* create_func_node(char* name, char* return_type) {
    FuncDeclNode* func = malloc(sizeof(FuncDeclNode));
    if (!func) return NULL;
    
    func->base.type = NODE_FUNC_DECL;
    func->name = name;
    func->return_type = return_type; // caller decides if null explicitly
    func->param_names = NULL;
    func->param_types = NULL;
    func->param_count = 0;
    func->body = NULL;
    
    return (AstNode*)func;
}

AstNode* create_block_node() {
    BlockNode* block = malloc(sizeof(BlockNode));
    if (!block) return NULL;

    block->base.type = NODE_BLOCK;
    block->statements = NULL;
    block->statement_count = 0;

    return (AstNode*)block;
}

AstNode* create_var_node(char* name, char* type, AstNode* initializer) {
    VarDeclNode* var = malloc(sizeof(VarDeclNode));
    if (!var) return NULL;

    var->base.type = NODE_VAR_DECL;
    var->name = name;
    var->type = type;
    var->initializer = initializer;
    var->is_mutable = false;
    var->is_reference = false;

    return (AstNode*)var;
}

AstNode* create_array_node(char* name, char* type, int size, AstNode** elements) {
    // Check array size, must be valid
    if (size <= 0) {
        printf("ERROR: Invalid array size: %d", size);
        return NULL;
    }

    ArrayDeclNode* array = malloc(sizeof(ArrayDeclNode));
    if (!array) return NULL;

    array->base.type = NODE_ARRAY_DECL;
    array->name = name;
    array->type = type;
    array->list.count = size;
    array->list.elements = elements; // Elements could be an array of nulls or of expressions. We don't care here, either way.

    // TODO: the commented section can be moved to parser.

    // We have two ways to declare/initialize an array:
    //      1. {}string my_str = array{"hey", "there", ...} - size is inferred
    //      2. {}string my_str = {5}array 
    // For the first, we expect the param elements to be a full array of expressions (the element could be 5+x for example)
    // For the second, expect elements to be passed in as null. In that case, create an empty array of that size.
    // if (elements) {
    //     if (sizeof(elements) != (size * sizeof(AstNode*)) ) {
    //         printf("ERROR: Elements provided for the array is not of the correct size. \n\tExpected '%d' but got '%d'", size * sizeof(AstNode*), sizeof(elements));
    //         return NULL;
    //     }
    //     array->list.elements = elements;
    // }
    // else { // Else - elements not provided
    //     // Allocate the space for our one-dimensional array of AstNode pointers
    //     array->list.elements = malloc(size * sizeof(AstNode*));
    //     if (!array->list.elements) {
    //         printf("Failed to allocate memory space for array elements, size: %d", size);
    //         free_ast_node(array);
    //         return NULL;
    //     }

    //     // Allocate each element with a null AstNode
    //     for (int i = 0; i < size; i++) {
    //         array->list.elements[i] = malloc(sizeof(AstNode));
    //     }
    // }

    return (AstNode*)array;
}

AstNode* create_list_node(char* name, char* type, int capacity, AstNode** elements, int elements_count) {
    if (capacity < 0) {
        printf("ERROR: Invalid list capacity: %d", capacity);
        return NULL;
    }
    if (capacity < elements_count) {
        printf("ERROR: Too many elements (supplied: %d) supplied to list (capacity: %d)", elements_count, capacity);
        return NULL;
    }

    ListDeclNode* new_list = malloc(sizeof(ArrayDeclNode));
    if (!new_list) return NULL;

    new_list->base.type = NODE_LIST_DECL;
    new_list->name = name;
    new_list->type = type;
    new_list->list.capacity = capacity;
    new_list->list.count = elements_count;
    new_list->list.elements = elements;

    // TODO: the commented section can be moved to parser.

    // Number of elements provided (REDUNDANT AFTER )
    // int size_elements = sizeof(elements) / sizeof(elements[0]);

    // Find out how many elements have been supplied.
    // for (int i = 0; i < size_elements; i++) {
    //     if (elements[i] != NULL) {
    //         new_list->list.count++;
    //     }
    // }

    // // If count equals capacity, just set them equal. Otherwise, we allocate the space and copy as many elements as possible.
    // if (new_list->list.count == size_elements) {
    //     new_list->list.elements = elements;
    // }
    // else {
    //     // Allocate array of size capacity
    //     new_list->list.elements = malloc(capacity*sizeof(AstNode));
    //     if (!new_list->list.elements) {
    //         printf("Failed to allocate memory space for list elements, capacity: %d", capacity);
    //         free_ast_node(new_list);
    //         return NULL;
    //     }

    //     // Copy any elements provided (remaining spots will be null pointers)
    //     for (int i = 0; i < size_elements; i++) {
    //         new_list->list.elements[i] = elements[i];
    //     }
    // }

    return (AstNode*)new_list;
}
AstNode* create_tuple_node(char* name, int size, AstNode** elements) {
    if (size <= 0) {
        printf("ERROR: Invalid tuple size: %d", size);
        return NULL;
    }
    TupleDeclNode* tuple = malloc(sizeof(TupleDeclNode));

    tuple->base.type = NODE_TUPLE_DECL;
    tuple->name = name;
    tuple->list.elements = elements;
    tuple->list.count = size;

    return (AstNode*)tuple;
}

AstNode* create_set_node(char* name, char* type, int capacity, AstNode** elements, int elements_count) {
    if (capacity <= 0) {
        printf("ERROR: Invalid tuple capacity: %d", capacity);
        return NULL;
    }
    if (capacity < elements_count) {
        printf("ERROR: Too many elements (supplied: %d) supplied to set (capacity: %d)", elements_count, capacity);
        return NULL;
    }

    SetDeclNode* set = malloc(sizeof(SetDeclNode));

    set->name = name;
    set->type = type;
    set->list.capacity = capacity;
    set->list.count = elements_count;

    // Hash provided elements into our set
    for (int i = 0; i < elements_count; i++) {
        AstNode* key = elements[i];

        // TODO: Implement hash function for int. Strings must be turned into ints: https://stackoverflow.com/a/2624210
        set->list.elements[hash(key)] = true;
    }

    return (AstNode*)set;
}

AstNode* create_map_node(char* name, char* key_type, char* value_type, int capacity, AstNode*** elements, int elements_count) {
    if (capacity != NULL && capacity <= 0) {
        printf("ERROR: Invalid map capacity: %d", capacity);
        return NULL;
    }
    MapDeclNode* map = malloc(sizeof(MapDeclNode));

    map->base.type = NODE_MAP_DECL;
    map->name = name;
    map->key_type = key_type;
    map->value_type = value_type;
    map->load_factor = 0.75; // Load factor. Double array size when load factor =< count/capacity.
    map->list.capacity = capacity;
    map->list.count = elements_count;

    // Hash provided elements into our map
    for (int i = 0; i < elements_count; i++) {
        AstNode* key = elements[i][0];
        AstNode* value = elements[i][1];

        // TODO: Implement hash function for int. Strings must be turned into ints: https://stackoverflow.com/a/2624210
        map->list.elements[hash(key)] = value;
    }

    return (AstNode*)map;
}

AstNode* create_expr_node(AstNode* expression) {
    ExprStmtNode* expr = malloc(sizeof(ExprStmtNode));
    if (!expr) return NULL;

    expr->base.type = NODE_EXPR_STMT;
    expr->expression = expression;

    return (AstNode*)expr;
}

AstNode* create_return_node() {
    ReturnStmtNode* ret_stmt = malloc(sizeof(ReturnStmtNode));
    if (!ret_stmt) return NULL;

    ret_stmt->base.type = NODE_RETURN_STMT;
    ret_stmt->value = NULL;

    return (AstNode*)ret_stmt;
}

AstNode* create_assign_node() {
    AssignExprNode* assign = malloc(sizeof(AssignExprNode));
    if (!assign) return NULL;

    assign->base.type = NODE_ASSIGN_EXPR;
    assign->target = NULL;
    assign->value = NULL;

    return (AstNode*)assign;
}

AstNode* create_binexpr_node(AstNode* left, TokenType operator, AstNode* right) {
    BinaryExprNode* binexpr = malloc(sizeof(BinaryExprNode));
    if (!binexpr) return NULL;

    binexpr->base.type = NODE_BINARY_EXPR;
    binexpr->left = left;
    binexpr->operator = operator;
    binexpr->right = right;

    return (AstNode*)binexpr;
}

AstNode* create_ident_node(char* name) {
    IdentExprNode* ident = malloc(sizeof(IdentExprNode));
    if (!ident) return NULL;

    ident->base.type = NODE_IDENT_EXPR;
    ident->name = name;

    return(AstNode*)ident;
}

AstNode* create_literal_node(Token* token) {
    LiteralExprNode* literal = malloc(sizeof(LiteralExprNode));
    if (!literal) return NULL;
    
    literal->base.type = NODE_LITERAL_EXPR;
    literal->literal_type = token->type;
    
    // Copy the value from the token
    switch (token->type) {
        case INT_LITERAL:
            literal->value.int_value = token->value.int_value;
            break;
        case FLOAT_LITERAL:
            literal->value.float_value = token->value.float_value;
            break;
        case CHAR_LITERAL:
            literal->value.char_value = token->value.char_value;
            break;
        case BOOL_LITERAL:
            literal->value.bool_value = token->value.bool_value;
            break;
        case STRING_LITERAL:
            literal->value.string_value = my_strdup(token->value.string_value);
            break;
        default:
            free(literal);
            return NULL;
    }
    
    return (AstNode*)literal;
}

AstNode* create_call_node(AstNode* function) {
    CallExprNode* call = malloc(sizeof(CallExprNode));
    if (!call) return NULL;

    call->base.type = NODE_CALL_EXPR;
    call->function = function;
    call->arguments = NULL;
    call->argument_count = 0;

    return (AstNode*)call;
}

void dump_ast(AstNode* node, int indent) {
    if (!node) {
        printf("%*sNULL\n", indent, "");
        return;
    }
    
    // Print indentation
    char indent_str[256] = {0};
    for (int i = 0; i < indent; i++) {
        indent_str[i] = ' ';
    }
    
    // Handle each node type
    switch (node->type) {
        case NODE_PROGRAM: {
            ProgramNode* program = (ProgramNode*)node;
            printf("%s[PROGRAM] statements: %d\n", indent_str, program->statement_count);
            for (int i = 0; i < program->statement_count; i++) {
                dump_ast(program->statements[i], indent + 2);
            }
            break;
        }
        case NODE_FUNC_DECL: {
            FuncDeclNode* func = (FuncDeclNode*)node;
            printf("%s[FUNCTION] name: '%s', return_type: %s, params: %d\n", 
                indent_str, 
                func->name, 
                func->return_type ? func->return_type : "void", 
                func->param_count);
            
            // Print parameters
            for (int i = 0; i < func->param_count; i++) {
                printf("%s  [PARAM] %s: %s\n", 
                    indent_str, 
                    func->param_names[i], 
                    func->param_types[i]);
            }
            
            // Print function body
            if (func->body) {
                printf("%s  [BODY]\n", indent_str);
                dump_ast(func->body, indent + 4);
            } else {
                printf("%s  [BODY] NULL\n", indent_str);
            }
            break;
        }
        case NODE_BLOCK: {
            BlockNode* block = (BlockNode*)node;
            printf("%s[BLOCK] statements: %d\n", indent_str, block->statement_count);
            for (int i = 0; i < block->statement_count; i++) {
                dump_ast(block->statements[i], indent + 2);
            }
            break;
        }
        case NODE_VAR_DECL: {
            VarDeclNode* var = (VarDeclNode*)node;
            printf("%s[VAR] name: '%s', type: %s, mutable: %s, ref: %s\n",
                indent_str,
                var->name,
                var->type,
                var->is_mutable ? "true" : "false",
                var->is_reference ? "true" : "false");
            
            if (var->initializer) {
                printf("%s  [INIT]\n", indent_str);
                dump_ast(var->initializer, indent + 4);
            }
            break;
        }
        case NODE_EXPR_STMT: {
            ExprStmtNode* expr = (ExprStmtNode*)node;
            printf("%s[EXPR_STMT]\n", indent_str);
            if (expr->expression) {
                dump_ast(expr->expression, indent + 2);
            }
            break;
        }
        case NODE_RETURN_STMT: {
            ReturnStmtNode* ret = (ReturnStmtNode*)node;
            printf("%s[RETURN]\n", indent_str);
            if (ret->value) {
                dump_ast(ret->value, indent + 2);
            }
            break;
        }
        case NODE_ASSIGN_EXPR: {
            AssignExprNode* assign = (AssignExprNode*)node;
            printf("%s[ASSIGN]\n", indent_str);
            printf("%s  [TARGET]\n", indent_str);
            dump_ast(assign->target, indent + 4);
            printf("%s  [VALUE]\n", indent_str);
            dump_ast(assign->value, indent + 4);
            break;
        }
        case NODE_BINARY_EXPR: {
            BinaryExprNode* bin = (BinaryExprNode*)node;
            printf("%s[BINARY] operator: %s\n", indent_str, token_type_to_string(bin->operator));
            printf("%s  [LEFT]\n", indent_str);
            dump_ast(bin->left, indent + 4);
            printf("%s  [RIGHT]\n", indent_str);
            dump_ast(bin->right, indent + 4);
            break;
        }
        case NODE_IDENT_EXPR: {
            IdentExprNode* ident = (IdentExprNode*)node;
            printf("%s[IDENT] '%s'\n", indent_str, ident->name);
            break;
        }
        case NODE_LITERAL_EXPR: {
            LiteralExprNode* literal = (LiteralExprNode*)node;
            printf("%s[LITERAL] type: %s, value: ", indent_str, token_type_to_string(literal->literal_type));
            
            // Print the value based on its type
            switch (literal->literal_type) {
                case INT_LITERAL:
                    printf("%d\n", literal->value.int_value);
                    break;
                case FLOAT_LITERAL:
                    printf("%f\n", literal->value.float_value);
                    break;
                case CHAR_LITERAL:
                    printf("'%c'\n", literal->value.char_value);
                    break;
                case STRING_LITERAL:
                    printf("\"%s\"\n", literal->value.string_value);
                    break;
                case BOOL_LITERAL:
                    printf("%s\n", literal->value.bool_value ? "true" : "false");
                    break;
                default:
                    printf("unknown\n");
                    break;
            }
            break;
        }
        case NODE_CALL_EXPR: {
            CallExprNode* call = (CallExprNode*)node;
            printf("%s[CALL] args: %d\n", indent_str, call->argument_count);
            printf("%s  [FUNCTION]\n", indent_str);
            dump_ast(call->function, indent + 4);
            
            for (int i = 0; i < call->argument_count; i++) {
                printf("%s  [ARG %d]\n", indent_str, i);
                dump_ast(call->arguments[i], indent + 4);
            }
            break;
        }
        default:
            printf("%s[UNKNOWN NODE TYPE: %d]\n", indent_str, node->type);
            break;
    }
}

bool check_semantics(AstNode* ast, struct SymbolTable* symbol_table) {
    // This is a stub implementation that always passes
    return true;
}