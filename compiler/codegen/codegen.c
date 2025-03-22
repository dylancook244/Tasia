#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#include <llvm-c/Core.h>       // Core LLVM functionality
#include <llvm-c/Analysis.h>   // For analysis passes
#include <llvm-c/BitWriter.h>  // For writing bitcode to file
#include <llvm-c/Target.h>     // For target-specific code gen

#include "../ast/ast.h"
#include "../symbol_table/symbol_table.h"
#include "codegen.h"

CodegenContext* create_codegen(const char* module_name) {
    CodegenContext* context = malloc(sizeof(CodegenContext));
    if (!context) return NULL;

    context->context = LLVMContextCreate();
    context->module = LLVMModuleCreateWithNameInContext(module_name, context->context);
    context->builder = LLVMCreateBuilderInContext(context->context);
    
    // Initialize variable mappings
    context->var_mappings.names = NULL;
    context->var_mappings.values = NULL;
    context->var_mappings.count = 0;
    context->var_mappings.capacity = 0;
    
    return context;
}

void generate_code(AstNode* ast, struct SymbolTable* symbol_table, const char* output_file) {
    printf("Generating LLVM IR to %s\n", output_file);
    
    // Initialize codegen context
    CodegenContext* context = create_codegen("tasia_module");
    if (!context) {
        printf("Error: Failed to initialize code generation context\n");
        return;
    }
    
    // Set the symbol table in the context
    context->symbol_table = symbol_table;
    
    // Generate LLVM IR
    LLVMModuleRef module = generate_ir(context, ast);
    if (!module) {
        printf("Error: Failed to generate IR\n");
        free_codegen(context);
        return;
    }
    
    // Dump the module's IR to stdout for debugging
    printf("\n=== LLVM IR DUMP ===\n");
    LLVMDumpModule(module);
    printf("=== END OF COMPILER DUMP ===\n\n");
    
    // Write the IR to a file
    char* error = NULL;
    LLVMPrintModuleToFile(module, output_file, &error);
    if (error) {
        printf("Error: %s\n", error);
        LLVMDisposeMessage(error);
    }
    
    // Also write the bitcode to file (for llc)
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
                default:
                    // Either handle or ignore other statement types
                    break;
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
        } else if (strcmp(func->return_type, "float") == 0) {
            return_type = LLVMFloatTypeInContext(context->context);
        } else if (strcmp(func->return_type, "bool") == 0) {
            return_type = LLVMInt1TypeInContext(context->context);
        } else if (strcmp(func->return_type, "char") == 0) {
            return_type = LLVMInt8TypeInContext(context->context);
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
            } else if (strcmp(func->param_types[i], "float") == 0) {
                param_types[i] = LLVMFloatTypeInContext(context->context);
            } else if (strcmp(func->param_types[i], "bool") == 0) {
                param_types[i] = LLVMInt1TypeInContext(context->context);
            } else if (strcmp(func->param_types[i], "char") == 0) {
                param_types[i] = LLVMInt8TypeInContext(context->context);
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
        
        // Add parameters to symbol table
        for (int i = 0; i < func->param_count; i++) {
            LLVMValueRef param = LLVMGetParam(llvm_func, i);
            LLVMSetValueName2(param, func->param_names[i], strlen(func->param_names[i]));
            
            // Store the parameter value in an alloca
            LLVMValueRef alloca = LLVMBuildAlloca(
                context->builder,
                LLVMTypeOf(param),
                func->param_names[i]
            );
            LLVMBuildStore(context->builder, param, alloca);
            
            // Add to our variable mapping
            add_variable_mapping(context, func->param_names[i], alloca);
        }
        
        // Generate code for function body
        if (func->body->type == NODE_BLOCK) {
            BlockNode* block = (BlockNode*)func->body;
            for (int i = 0; i < block->statement_count; i++) {
                generate_statement(context, block->statements[i]);
            }
        }
        
        // If control reaches the end of the function without a return,
        // add a default return instruction
        if (!LLVMGetBasicBlockTerminator(LLVMGetInsertBlock(context->builder))) {
            if (return_type == LLVMVoidTypeInContext(context->context)) {
                LLVMBuildRetVoid(context->builder);
            } else {
                // Add a default return value based on type
                LLVMValueRef default_val;
                if (return_type == LLVMInt32TypeInContext(context->context)) {
                    default_val = LLVMConstInt(return_type, 0, 0);
                } else if (return_type == LLVMFloatTypeInContext(context->context)) {
                    default_val = LLVMConstReal(return_type, 0.0);
                } else if (return_type == LLVMInt1TypeInContext(context->context)) {
                    default_val = LLVMConstInt(return_type, 0, 0);
                } else {
                    default_val = LLVMConstNull(return_type);
                }
                LLVMBuildRet(context->builder, default_val);
            }
        }
    }
    
    return llvm_func;
}

LLVMValueRef generate_statement(CodegenContext* context, AstNode* stmt) {
    if (!stmt) return NULL;
    
    switch (stmt->type) {
        case NODE_VAR_DECL: {
            VarDeclNode* var = (VarDeclNode*)stmt;
            
            // Determine variable type
            LLVMTypeRef var_type;
            if (strcmp(var->type, "int") == 0) {
                var_type = LLVMInt32TypeInContext(context->context);
            } else if (strcmp(var->type, "float") == 0) {
                var_type = LLVMFloatTypeInContext(context->context);
            } else if (strcmp(var->type, "bool") == 0) {
                var_type = LLVMInt1TypeInContext(context->context);
            } else if (strcmp(var->type, "char") == 0) {
                var_type = LLVMInt8TypeInContext(context->context);
            } else {
                printf("Error: Unsupported variable type: %s\n", var->type);
                return NULL;
            }
            
            // Create an alloca for the variable
            LLVMValueRef alloca = LLVMBuildAlloca(context->builder, var_type, var->name);
            
            // Add to our variable mapping
            add_variable_mapping(context, var->name, alloca);
            
            // Initialize if there's an initializer
            if (var->initializer) {
                LLVMValueRef init_val = generate_expression(context, var->initializer);
                if (init_val) {
                    LLVMBuildStore(context->builder, init_val, alloca);
                }
            }
            
            return alloca;
        }
        
        case NODE_EXPR_STMT: {
            ExprStmtNode* expr_stmt = (ExprStmtNode*)stmt;
            return generate_expression(context, expr_stmt->expression);
        }
        
        case NODE_RETURN_STMT: {
            ReturnStmtNode* ret = (ReturnStmtNode*)stmt;
            
            if (ret->value) {
                LLVMValueRef ret_val = generate_expression(context, ret->value);
                return LLVMBuildRet(context->builder, ret_val);
            } else {
                return LLVMBuildRetVoid(context->builder);
            }
        }
        
        case NODE_BLOCK: {
            BlockNode* block = (BlockNode*)stmt;
            LLVMValueRef last_val = NULL;
            
            for (int i = 0; i < block->statement_count; i++) {
                last_val = generate_statement(context, block->statements[i]);
            }
            
            return last_val;
        }
        
        default:
            printf("Error: Unsupported statement type: %d\n", stmt->type);
            return NULL;
    }
}

LLVMValueRef generate_expression(CodegenContext* context, AstNode* expr) {
    if (!expr) return NULL;
    
    switch (expr->type) {
        case NODE_IDENT_EXPR: {
            IdentExprNode* ident = (IdentExprNode*)expr;
            
            // Look up variable in our mapping
            LLVMValueRef var = get_variable_mapping(context, ident->name);
            if (!var) {
                printf("Error: Undefined variable: %s\n", ident->name);
                return NULL;
            }
            
            // If it's an alloca, we need to load it
            if (LLVMIsAAllocaInst(var)) {
                return LLVMBuildLoad2(
                    context->builder,
                    LLVMGetAllocatedType(var),
                    var,
                    "load"
                );
            }
            
            return var;
        }
        
        case NODE_LITERAL_EXPR: {
            LiteralExprNode* literal = (LiteralExprNode*)expr;
            
            switch (literal->literal_type) {
                case INT_LITERAL:
                    return LLVMConstInt(
                        LLVMInt32TypeInContext(context->context),
                        literal->value.int_value,
                        0  // not signed
                    );
                
                case FLOAT_LITERAL:
                    return LLVMConstReal(
                        LLVMFloatTypeInContext(context->context),
                        literal->value.float_value
                    );
                
                case BOOL_LITERAL:
                    return LLVMConstInt(
                        LLVMInt1TypeInContext(context->context),
                        literal->value.bool_value ? 1 : 0,
                        0  // not signed
                    );
                
                case CHAR_LITERAL:
                    return LLVMConstInt(
                        LLVMInt8TypeInContext(context->context),
                        literal->value.char_value,
                        0  // not signed
                    );
                
                case STRING_LITERAL: {
                    // Create a string constant as a global
                    LLVMValueRef str = LLVMBuildGlobalStringPtr(
                        context->builder,
                        literal->value.string_value,
                        "str"
                    );
                    return str;
                }
                
                default:
                    printf("Error: Unsupported literal type: %d\n", literal->literal_type);
                    return NULL;
            }
        }
        
        case NODE_BINARY_EXPR: {
            BinaryExprNode* bin = (BinaryExprNode*)expr;
            
            LLVMValueRef left = generate_expression(context, bin->left);
            LLVMValueRef right = generate_expression(context, bin->right);
            
            if (!left || !right) return NULL;
            
            switch (bin->operator) {
                case ADD:
                    if (LLVMTypeOf(left) == LLVMFloatTypeInContext(context->context)) {
                        return LLVMBuildFAdd(context->builder, left, right, "faddtmp");
                    } else {
                        return LLVMBuildAdd(context->builder, left, right, "addtmp");
                    }
                
                case SUBTRACT:
                    if (LLVMTypeOf(left) == LLVMFloatTypeInContext(context->context)) {
                        return LLVMBuildFSub(context->builder, left, right, "fsubtmp");
                    } else {
                        return LLVMBuildSub(context->builder, left, right, "subtmp");
                    }
                
                case MULTIPLY:
                    if (LLVMTypeOf(left) == LLVMFloatTypeInContext(context->context)) {
                        return LLVMBuildFMul(context->builder, left, right, "fmultmp");
                    } else {
                        return LLVMBuildMul(context->builder, left, right, "multmp");
                    }
                
                case QUOTIENT:
                    if (LLVMTypeOf(left) == LLVMFloatTypeInContext(context->context)) {
                        return LLVMBuildFDiv(context->builder, left, right, "fdivtmp");
                    } else {
                        // For signed division
                        return LLVMBuildSDiv(context->builder, left, right, "divtmp");
                    }
                
                case ASSIGN: {
                    // Special case for assignment
                    if (bin->left->type != NODE_IDENT_EXPR) {
                        printf("Error: Left-hand side of assignment must be a variable\n");
                        return NULL;
                    }
                    
                    IdentExprNode* ident = (IdentExprNode*)bin->left;
                    LLVMValueRef var = get_variable_mapping(context, ident->name);
                    
                    if (!var) {
                        printf("Error: Undefined variable: %s\n", ident->name);
                        return NULL;
                    }
                    
                    // Store the right value into the variable
                    return LLVMBuildStore(context->builder, right, var);
                }
                
                default:
                    printf("Error: Unsupported binary operator: %d\n", bin->operator);
                    return NULL;
            }
        }
        
        case NODE_CALL_EXPR: {
            CallExprNode* call = (CallExprNode*)expr;
            
            // Get the function
            LLVMValueRef func;
            if (call->function->type == NODE_IDENT_EXPR) {
                IdentExprNode* ident = (IdentExprNode*)call->function;
                func = LLVMGetNamedFunction(context->module, ident->name);
                
                if (!func) {
                    printf("Error: Undefined function: %s\n", ident->name);
                    return NULL;
                }
            } else {
                printf("Error: Function call must reference a function name\n");
                return NULL;
            }
            
            // Check argument count
            if (LLVMCountParams(func) != call->argument_count) {
                printf("Error: Function called with wrong number of arguments\n");
                return NULL;
            }
            
            // Generate code for each argument
            LLVMValueRef* args = NULL;
            if (call->argument_count > 0) {
                args = malloc(sizeof(LLVMValueRef) * call->argument_count);
                
                for (int i = 0; i < call->argument_count; i++) {
                    args[i] = generate_expression(context, call->arguments[i]);
                    if (!args[i]) {
                        free(args);
                        return NULL;
                    }
                }
            }
            
            // Call the function
            LLVMValueRef call_expr = LLVMBuildCall2(
                context->builder,
                LLVMGetElementType(LLVMTypeOf(func)),
                func,
                args,
                call->argument_count,
                ""
            );
            
            // Free args array if allocated
            if (args) free(args);
            
            return call_expr;
        }
        
        default:
            printf("Error: Unsupported expression type: %d\n", expr->type);
            return NULL;
    }
}

void add_variable_mapping(CodegenContext* context, const char* name, LLVMValueRef value) {
    // Initialize if needed
    if (context->var_mappings.capacity == 0) {
        context->var_mappings.capacity = 16;
        context->var_mappings.names = malloc(sizeof(char*) * context->var_mappings.capacity);
        context->var_mappings.values = malloc(sizeof(LLVMValueRef) * context->var_mappings.capacity);
        context->var_mappings.count = 0;
    }
    
    // Resize if needed
    if (context->var_mappings.count >= context->var_mappings.capacity) {
        context->var_mappings.capacity *= 2;
        context->var_mappings.names = realloc(
            context->var_mappings.names, 
            sizeof(char*) * context->var_mappings.capacity
        );
        context->var_mappings.values = realloc(
            context->var_mappings.values, 
            sizeof(LLVMValueRef) * context->var_mappings.capacity
        );
    }
    
    // Add the mapping
    context->var_mappings.names[context->var_mappings.count] = strdup(name);
    context->var_mappings.values[context->var_mappings.count] = value;
    context->var_mappings.count++;
}

LLVMValueRef get_variable_mapping(CodegenContext* context, const char* name) {
    for (int i = 0; i < context->var_mappings.count; i++) {
        if (strcmp(context->var_mappings.names[i], name) == 0) {
            return context->var_mappings.values[i];
        }
    }
    
    return NULL;  // Not found
}

void free_codegen(CodegenContext* context) {
    if (context) {
        // Free variable mappings
        for (int i = 0; i < context->var_mappings.count; i++) {
            free(context->var_mappings.names[i]);
        }
        free(context->var_mappings.names);
        free(context->var_mappings.values);
        
        // Free LLVM objects
        LLVMDisposeBuilder(context->builder);
        LLVMDisposeModule(context->module);
        LLVMContextDispose(context->context);
        free(context);
    }
}