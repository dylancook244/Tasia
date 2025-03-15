#ifndef __TYPE_CHECKER_H__
#define __TYPE_CHECKER_H__

#include "SymbolTable.h"
#include "Types.h"
#include "BorrowChecker.h"
#include "ast/Program.h"
#include <vector>
#include <string>
#include <memory>
#include <unordered_map>

// The type checker is responsible for:
// - Type checking expressions and statements
// - Building symbol tables
// - Inferring types where possible
// - Coordinating with the borrow checker
class TypeChecker {
private:
    SymbolTable symbolTable;
    TypeRegistry typeRegistry;
    BorrowChecker borrowChecker;
    
    std::vector<std::string> errors;
    
    // Maps for inferred types
    std::unordered_map<ExprAST*, std::shared_ptr<Type>> exprTypes;
    
    // Helper methods
    void addError(const std::string& msg, const SourceLocation& loc);
    std::shared_ptr<Type> inferType(ExprAST* expr);
    
    // Type checking methods for specific AST nodes
    std::shared_ptr<Type> checkNumberExpr(NumberExprAST* expr);
    std::shared_ptr<Type> checkVariableExpr(VariableExprAST* expr);
    std::shared_ptr<Type> checkBinaryExpr(BinaryExprAST* expr);
    std::shared_ptr<Type> checkCallExpr(CallExprAST* expr);
    std::shared_ptr<Type> checkBlockExpr(BlockExprAST* expr);
    
    bool checkFunction(FuncAST* func);
    bool checkFuncInterface(FuncInterfaceAST* funcInterface);
    bool checkStatement(StmtAST* stmt);
    
public:
    TypeChecker() : borrowChecker(symbolTable, typeRegistry) {}
    
    // Main entry point - check a program for type safety
    bool checkProgram(Program* program);
    
    // Get the inferred type of an expression
    std::shared_ptr<Type> getExprType(ExprAST* expr) {
        auto it = exprTypes.find(expr);
        if (it != exprTypes.end()) {
            return it->second;
        }
        return Type::createUnknown();
    }
    
    // Set the inferred type of an expression
    void setExprType(ExprAST* expr, std::shared_ptr<Type> type) {
        exprTypes[expr] = type;
    }
    
    // Access results
    const std::vector<std::string>& getErrors() const { return errors; }
    bool hasErrors() const { return !errors.empty() || borrowChecker.hasErrors(); }
    
    // Access the symbol table
    SymbolTable& getSymbolTable() { return symbolTable; }
    const SymbolTable& getSymbolTable() const { return symbolTable; }
    
    // Access the type registry
    TypeRegistry& getTypeRegistry() { return typeRegistry; }
    const TypeRegistry& getTypeRegistry() const { return typeRegistry; }
    
    // Access the borrow checker
    BorrowChecker& getBorrowChecker() { return borrowChecker; }
    const BorrowChecker& getBorrowChecker() const { return borrowChecker; }
};

// Implementation of key methods to demonstrate how the system works
void TypeChecker::addError(const std::string& msg, const SourceLocation& loc) {
    std::string errorMsg = loc.filename + ":" + std::to_string(loc.line) + ":" + 
                           std::to_string(loc.column) + ": error: " + msg;
    errors.push_back(errorMsg);
}

bool TypeChecker::checkProgram(Program* program) {
    if (!program) return false;
    
    // Check all function interfaces first (for forward references)
    for (const auto& func : program->getFunctions()) {
        if (!checkFuncInterface(func->getFuncInterface())) {
            return false;
        }
    }
    
    // Check all function bodies
    for (const auto& func : program->getFunctions()) {
        if (!checkFunction(func.get())) {
            return false;
        }
    }
    
    // Check all top-level statements
    for (const auto& stmt : program->getStatements()) {
        if (!checkStatement(stmt.get())) {
            return false;
        }
    }
    
    // Finally, run the borrow checker
    if (!borrowChecker.checkProgram(program)) {
        // Add borrow checker errors to our errors
        for (const auto& error : borrowChecker.getErrors()) {
            errors.push_back(error);
        }
        return false;
    }
    
    return !hasErrors();
}

std::shared_ptr<Type> TypeChecker::inferType(ExprAST* expr) {
    if (!expr) return Type::createUnknown();
    
    // Check if we've already inferred this type
    auto it = exprTypes.find(expr);
    if (it != exprTypes.end()) {
        return it->second;
    }
    
    // Dispatch to appropriate handler based on expression type
    if (auto* numExpr = dynamic_cast<NumberExprAST*>(expr)) {
        return checkNumberExpr(numExpr);
    } else if (auto* varExpr = dynamic_cast<VariableExprAST*>(expr)) {
        return checkVariableExpr(varExpr);
    } else if (auto* binExpr = dynamic_cast<BinaryExprAST*>(expr)) {
        return checkBinaryExpr(binExpr);
    } else if (auto* callExpr = dynamic_cast<CallExprAST*>(expr)) {
        return checkCallExpr(callExpr);
    } else if (auto* blockExpr = dynamic_cast<BlockExprAST*>(expr)) {
        return checkBlockExpr(blockExpr);
    }
    
    return Type::createUnknown();
}

std::shared_ptr<Type> TypeChecker::checkNumberExpr(NumberExprAST* expr) {
    // All numeric literals are considered floats for simplicity
    // A real implementation would distinguish between integer and float literals
    auto type = typeRegistry.getFloat();
    setExprType(expr, type);
    return type;
}

std::shared_ptr<Type> TypeChecker::checkVariableExpr(VariableExprAST* expr) {
    const std::string& varName = expr->getName();
    Symbol* symbol = symbolTable.lookup(varName);
    
    if (!symbol) {
        addError("Use of undeclared variable '" + varName + "'", expr->getLocation());
        return Type::createUnknown();
    }
    
    // Record variable usage
    symbol->addUsage(expr->getLocation().filename, expr->getLocation().line, 
                     expr->getLocation().column);
    
    setExprType(expr, symbol->type);
    return symbol->type;
}

std::shared_ptr<Type> TypeChecker::checkBinaryExpr(BinaryExprAST* expr) {
    std::shared_ptr<Type> lhsType = inferType(expr->getLHS());
    std::shared_ptr<Type> rhsType = inferType(expr->getRHS());
    
    // Make sure both sides have valid types
    if (lhsType->getKind() == TypeKind::Unknown || rhsType->getKind() == TypeKind::Unknown) {
        return Type::createUnknown();
    }
    
    char op = expr->getOperator();
    std::shared_ptr<Type> resultType;
    
    if (op == '=') {
        // Assignment operator
        // Check if LHS is assignable (must be a variable or field access)
        auto* varExpr = dynamic_cast<VariableExprAST*>(expr->getLHS());
        if (!varExpr) {
            addError("Left side of assignment must be assignable", expr->getLocation());
            return Type::createUnknown();
        }
        
        // Check if types are compatible
        Symbol* symbol = symbolTable.lookup(varExpr->getName());
        if (!symbol) {
            // Should have been caught in checkVariableExpr, but just in case
            addError("Use of undeclared variable", expr->getLocation());
            return Type::createUnknown();
        }
        
        // For assignment, check mutability
        if (!symbol->type->isMutable()) {
            addError("Cannot assign to immutable variable", expr->getLocation());
            return Type::createUnknown();
        }
        
        // Check type compatibility
        if (!rhsType->isCompatibleWith(symbol->type.get())) {
            addError("Cannot assign incompatible type to variable", expr->getLocation());
            return Type::createUnknown();
        }
        
        // Assignment evaluates to the assigned value
        resultType = rhsType;
    } else {
        // Arithmetic operators
        if (op == '+' || op == '-' || op == '*' || op == '/') {
            // Both sides must be numeric
            if ((lhsType->getKind() != TypeKind::Integer && lhsType->getKind() != TypeKind::Float) ||
                (rhsType->getKind() != TypeKind::Integer && rhsType->getKind() != TypeKind::Float)) {
                addError("Arithmetic operators require numeric operands", expr->getLocation());
                return Type::createUnknown();
            }
            
            // If either side is float, result is float
            if (lhsType->getKind() == TypeKind::Float || rhsType->getKind() == TypeKind::Float) {
                resultType = typeRegistry.getFloat();
            } else {
                resultType = typeRegistry.getInt();
            }
        } else if (op == '<' || op == '>' || op == '=' || op == '!') {
            // Comparison operators
            if (!lhsType->isCompatibleWith(rhsType.get())) {
                addError("Cannot compare incompatible types", expr->getLocation());
                return Type::createUnknown();
            }
            
            resultType = typeRegistry.getBool();
        } else {
            addError("Unsupported binary operator", expr->getLocation());
            return Type::createUnknown();
        }
    }
    
    setExprType(expr, resultType);
    return resultType;
}

std::shared_ptr<Type> TypeChecker::checkCallExpr(CallExprAST* expr) {
    const std::string& funcName = expr->getCallee();
    
    // Look up the function in the symbol table
    Symbol* funcSymbol = symbolTable.lookup(funcName);
    if (!funcSymbol) {
        addError("Call to undefined function '" + funcName + "'", expr->getLocation());
        return Type::createUnknown();
    }
    
    // Check that it's a function type
    if (funcSymbol->type->getKind() != TypeKind::Function) {
        addError("Called object is not a function", expr->getLocation());
        return Type::createUnknown();
    }
    
    // Check the argument count
    FunctionType* funcType = static_cast<FunctionType*>(funcSymbol->type.get());
    if (expr->getArgs().size() != funcType->getParamTypes().size()) {
        addError("Function call has wrong number of arguments", expr->getLocation());
        return Type::createUnknown();
    }
    
    // Check each argument type
    for (size_t i = 0; i < expr->getArgs().size(); ++i) {
        std::shared_ptr<Type> argType = inferType(expr->getArgs()[i].get());
        if (!argType->isCompatibleWith(funcType->getParamTypes()[i].get())) {
            addError("Function argument type mismatch", expr->getLocation());
            return Type::createUnknown();
        }
    }
    
    // The call's type is the function's return type
    setExprType(expr, funcType->getReturnType());
    return funcType->getReturnType();
}

std::shared_ptr<Type> TypeChecker::checkBlockExpr(BlockExprAST* expr) {
    // Enter a new scope for the block
    ScopeNode* blockScope = symbolTable.enterScope(
        expr->getLocation().filename,
        expr->getLocation().line,
        expr->getLocation().column
    );
    
    // Check all expressions in the block
    std::shared_ptr<Type> lastType = typeRegistry.getVoid();
    for (const auto& subExpr : expr->getExpressions()) {
        lastType = inferType(subExpr.get());
    }
    
    // Exit block scope
    symbolTable.exitScope(
        expr->getLocation().line + expr->getExpressions().size(),  // Estimate
        expr->getLocation().column
    );
    
    // The type of a block is the type of its last expression
    setExprType(expr, lastType);
    return lastType;
}

bool TypeChecker::checkFuncInterface(FuncInterfaceAST* FuncInterface) {
    if (!FuncInterface) return false;
    
    // For simplicity, assume all arguments are doubles (float)
    std::vector<std::shared_ptr<Type>> paramTypes;
    for (size_t i = 0; i < FuncInterface->getArgs().size(); ++i) {
        paramTypes.push_back(typeRegistry.getFloat());
    }
    
    // Create a function type with float return type
    auto funcType = typeRegistry.createFunctionType(
        typeRegistry.getFloat(),
        paramTypes
    );
    
    // Add the function to the symbol table
    symbolTable.addSymbol(
        FuncInterface->getName(),
        funcType,
        FuncInterface->getLocation().filename,
        FuncInterface->getLocation().line,
        FuncInterface->getLocation().column
    );
    
    return true;
}

bool TypeChecker::checkFunction(FuncAST* func) {
    if (!func) return false;
    
    // Enter a new scope for the function
    ScopeNode* functionScope = symbolTable.enterScope(
        func->getLocation().filename,
        func->getLocation().line,
        func->getLocation().column
    );
    
    // Add function parameters to the symbol table
    FuncInterfaceAST* funcInterface = func->getFuncInterface();
    for (size_t i = 0; i < funcInterface->getArgs().size(); ++i) {
        symbolTable.addSymbol(
            funcInterface->getArgs()[i],
            typeRegistry.getFloat(),  // Assume all params are float
            funcInterface->getLocation().filename,
            funcInterface->getLocation().line,
            funcInterface->getLocation().column
        );
    }
    
    // Check the function body
    std::shared_ptr<Type> returnType = inferType(func->getBody());
    
    // Verify return type
    Symbol* funcSymbol = symbolTable.getRootScope()->lookup(funcInterface->getName());
    if (funcSymbol) {
        FunctionType* funcType = static_cast<FunctionType*>(funcSymbol->type.get());
        if (!returnType->isCompatibleWith(funcType->getReturnType().get())) {
            addError("Function return type mismatch", func->getBody()->getLocation());
            return false;
        }
    }
    
    // Exit function scope
    symbolTable.exitScope(
        func->getBody()->getLocation().line + 1,  // Just an estimate
        func->getBody()->getLocation().column
    );
    
    return true;
}

bool TypeChecker::checkStatement(StmtAST* stmt) {
    if (!stmt) return true;  // Empty statement is fine
    
    // Statements are just expressions for now
    inferType(stmt->getExpression());
    
    return !hasErrors();
}

#endif // __TYPE_CHECKER_H__