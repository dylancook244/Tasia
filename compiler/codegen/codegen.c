#include "codegen.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <llvm-c/Core.h>
#include <llvm-c/Analysis.h>
#include <llvm-c/BitWriter.h>
#include "../symbol_table/symbol_table.h"

CodegenContext* init_codegen(const char* module_name) {
    CodegenContext* context = malloc(sizeof(CodegenContext));
    if (!context) return NULL;

    context->context = LLVMContextCreate();
    context->module = LLVMModuleCreateWithNameInContext(module_name, context->context);
    context->builder = LLVMCreateBuilderInContext(context->context);
    
    // Initialize symbol table and other fields
    
    return context;
}

void generate_code(AstNode* ast, struct SymbolTable* symbol_table, const char* output_file) {
    printf("Generating LLVM IR to %s\n", output_file);
    
    // Initialize codegen context
    CodegenContext* context = init_codegen("tasia_module");
    if (!context) {
        printf("Error: Failed to initialize code generation context\n");
        return;
    }
    
    // Generate LLVM IR
    LLVMModuleRef module = generate_ir(context, ast);
    if (!module) {
        printf("Error: Failed to generate IR\n");
        free_codegen(context);
        return;
    }
    
    // Write the bitcode to file
    if (LLVMWriteBitcodeToFile(module, output_file) != 0) {
        printf("Error: Could not write bitcode to file\n");
    } else {
        printf("Successfully wrote LLVM bitcode to %s\n", output_file);
    }
    
    // Clean up
    free_codegen(context);
}

LLVMModuleRef generate_ir(CodegenContext* context, AstNode* ast) {
    if (!ast) return NULL;

    // Handle the program node (root of AST)
    if (ast->type == NODE_PROGRAM) {
        ProgramNode* program = (ProgramNode*)ast;
        
        // Generate code for each top-level statement
        for (int i = 0; i < program->statement_count; i++) {
            AstNode* stmt = program->statements[i];
            
            // Generate appropriate code based on statement type
            switch (stmt->type) {
                case NODE_FUNC_DECL:
                    generate_function(context, (FuncDeclNode*)stmt);
                    break;
                // Handle other top-level statements
            }
        }
    }
    
    return context->module;
}

LLVMValueRef generate_function(CodegenContext* context, FuncDeclNode* func) {
    // Get return type
    LLVMTypeRef return_type = LLVMVoidTypeInContext(context->context);
    if (func->return_type) {
        // Convert Tasia type to LLVM type
        if (strcmp(func->return_type, "int") == 0) {
            return_type = LLVMInt32TypeInContext(context->context);
        }
        // Add other type conversions
    }
    
    // Create parameter types array
    LLVMTypeRef* param_types = NULL;
    if (func->param_count > 0) {
        param_types = malloc(sizeof(LLVMTypeRef) * func->param_count);
        for (int i = 0; i < func->param_count; i++) {
            // Convert parameter types to LLVM types
            if (strcmp(func->param_types[i], "int") == 0) {
                param_types[i] = LLVMInt32TypeInContext(context->context);
            }
            // Add other type conversions
        }
    }
    
    // Create function type
    LLVMTypeRef func_type = LLVMFunctionType(
        return_type, 
        param_types, 
        func->param_count, 
        0  // Not variadic
    );
    
    // Check if this is the main function and rename it if needed
    const char* actual_name = func->name;
    if (strcmp(func->name, "main") == 0) {
        actual_name = "TASIA_ENTRY_FUNCTION";
    }
    
    // Create function with potentially renamed function
    LLVMValueRef llvm_func = LLVMAddFunction(context->module, actual_name, func_type);
    
    // Free parameter types if allocated
    if (param_types) free(param_types);
    
    // If there's a function body, generate code for it
    if (func->body) {
        // Create entry block
        LLVMBasicBlockRef entry = LLVMAppendBasicBlockInContext(
            context->context, 
            llvm_func, 
            "entry"
        );
        
        // Position builder at the end of entry block
        LLVMPositionBuilderAtEnd(context->builder, entry);
        
        // Push scope for function parameters
        
        // Add parameters to symbol table
        
        // Generate code for function body
        if (func->body->type == NODE_BLOCK) {
            BlockNode* block = (BlockNode*)func->body;
            for (int i = 0; i < block->statement_count; i++) {
                generate_statement(context, block->statements[i]);
            }
        }
        
        // If control reaches the end of the function without a return,
        // add a void return instruction for void functions
        if (return_type == LLVMVoidTypeInContext(context->context)) {
            LLVMBuildRetVoid(context->builder);
        }
        
        // Pop scope
    }
    
    return llvm_func;
}

// Implement other generation functions
LLVMValueRef generate_statement(CodegenContext* context, AstNode* stmt) {
    // Handle different statement types
    switch (stmt->type) {
        case NODE_VAR_DECL:
            // Generate variable declaration
            break;
        case NODE_EXPR_STMT: 
            // Generate expression statement
            break;
        case NODE_RETURN_STMT:
            // Generate return statement
            break;
        // Handle other statement types
    }
    
    return NULL;
}

LLVMValueRef generate_expression(CodegenContext* context, AstNode* expr) {
    // Handle different expression types
    switch (expr->type) {
        case NODE_BINARY_EXPR:
            // Generate binary expression
            break;
        case NODE_IDENT_EXPR:
            // Generate identifier expression
            break;
        case NODE_LITERAL_EXPR:
            // Generate literal expression
            break;
        // Handle other expression types
    }
    
    return NULL;
}

void free_codegen(CodegenContext* context) {
    if (context) {
        LLVMDisposeBuilder(context->builder);
        LLVMDisposeModule(context->module);
        LLVMContextDispose(context->context);
        free(context);
    }
}