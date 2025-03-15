#ifndef __CODE_GENERATOR_H__
#define __CODE_GENERATOR_H__

#include <llvm/IR/Module.h>
#include <llvm/IR/Function.h>
#include <llvm/IR/Type.h>
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/Verifier.h>
#include <map>
#include <memory>
#include <string>
#include <vector>

// Include all AST node types
#include "ast/NumberExprAST.h"
#include "ast/VariableExprAST.h"
#include "ast/BinaryExprAST.h"
#include "ast/CallExprAST.h"
#include "ast/FuncInterfaceAST.h"
#include "ast/FuncAST.h"
#include "ast/BlockExprAST.h"
#include "ast/DeclarationExprAST.h"
#include "ast/ReferenceExprAST.h"
#include "ast/DereferenceExprAST.h"
#include "ast/StmtAST.h"
#include "ast/Program.h"

class CodeGenerator {
private:
    std::unique_ptr<llvm::LLVMContext> context;
    std::unique_ptr<llvm::IRBuilder<>> builder;
    std::unique_ptr<llvm::Module> module;
    std::map<std::string, llvm::Value*> namedValues;
    
    // Error reporting
    std::vector<std::string> errors;
    
    // Helper to report errors
    void reportError(const std::string& msg, const SourceLocation& loc);

    void createMainWrapper();

    llvm::AllocaInst* CreateEntryBlockAlloca(llvm::Function* func, const std::string& varName);

    llvm::Value* logErrorV(const std::string& str);

public:
    CodeGenerator(const std::string& moduleName = "my_module") {
        // Initialize everything with proper ownership
        context = std::make_unique<llvm::LLVMContext>();
        module = std::make_unique<llvm::Module>(moduleName, *context);
        builder = std::make_unique<llvm::IRBuilder<>>(*context);
    }
    
    // No need for copy or move constructors in this basic implementation
    
    // Generate code for all AST node types
    llvm::Value* generateCode(ExprAST* expr);
    llvm::Value* generateCode(NumberExprAST* expr);
    llvm::Value* generateCode(VariableExprAST* expr);
    llvm::Value* generateCode(BinaryExprAST* expr);
    llvm::Value* generateCode(CallExprAST* expr);
    llvm::Value* generateCode(BlockExprAST* expr);
    llvm::Value* generateCode(StmtAST* stmt);
    llvm::Value* generateCode(DeclarationExprAST* expr);
    llvm::Value* generateCode(ReferenceExprAST* expr);
    llvm::Value* generateCode(DereferenceExprAST* expr);
    llvm::Function* generateCode(FuncInterfaceAST* funcInterface);
    llvm::Function* generateCode(FuncAST* func);
    
    // Generate code for an entire program
    bool generateCode(Program* program);
    
    // Access compiled module and errors
    llvm::Module* getModule() { return module.get(); }
    llvm::LLVMContext& getContext() { return *context; }
    const std::vector<std::string>& getErrors() const { return errors; }
    bool hasErrors() const { return !errors.empty(); }
    
    // Reset the generator for a new compilation
    void reset(const std::string& moduleName = "my_module") {
        namedValues.clear();
        errors.clear();
        // Create new module with the same context
        module = std::make_unique<llvm::Module>(moduleName, *context);
        // Create new builder with the same context
        builder = std::make_unique<llvm::IRBuilder<>>(*context);
    }
};

#endif // __CODE_GENERATOR_H__