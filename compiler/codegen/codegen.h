// codegen.h
#ifndef CODEGEN_H
#define CODEGEN_H

#include "../ast/ast.h"
#include <llvm-c/Core.h>
#include <llvm-c/Analysis.h>
#include <llvm-c/Target.h>
#include <llvm-c/BitWriter.h>

// Context holds the LLVM state during code generation
typedef struct {
    LLVMContextRef context;
    LLVMModuleRef module;
    LLVMBuilderRef builder;
    // Symbol table for tracking variables
    // ... other necessary fields
} CodegenContext;

// Initialize codegen context
CodegenContext* init_codegen(const char* module_name);

// Generate LLVM IR from AST
LLVMModuleRef generate_ir(CodegenContext* context, AstNode* ast);

// Generate function
LLVMValueRef generate_function(CodegenContext* context, FuncDeclNode* func);

// Generate expressions, statements, etc.
LLVMValueRef generate_expression(CodegenContext* context, AstNode* expr);
LLVMValueRef generate_statement(CodegenContext* context, AstNode* stmt);

void generate_code(AstNode* ast, struct SymbolTable* symbol_table, const char* output_file);

// Free codegen context
void free_codegen(CodegenContext* context);

#endif