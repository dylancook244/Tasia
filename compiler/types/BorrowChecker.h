#ifndef __BORROW_CHECKER_H__
#define __BORROW_CHECKER_H__

#include "SymbolTable.h"
#include "Types.h"
#include "ast/Program.h"
#include <vector>
#include <string>
#include <unordered_map>
#include <unordered_set>

// Forward declarations
class TypeChecker;

// Represents a constraint between lifetimes
struct LifetimeConstraint {
    enum class Kind {
        Equal,      // a == b
        Outlives,   // a outlives b (a: 'b)
        Contained   // a is contained in b
    };
    
    Lifetime a;
    Lifetime b;
    Kind kind;
    SourceLocation location; // Where this constraint originated
    
    LifetimeConstraint(const Lifetime& a, const Lifetime& b, Kind kind, const SourceLocation& loc)
        : a(a), b(b), kind(kind), location(loc) {}
    
    std::string toString() const {
        std::string kindStr;
        switch (kind) {
            case Kind::Equal: kindStr = " == "; break;
            case Kind::Outlives: kindStr = " outlives "; break;
            case Kind::Contained: kindStr = " contained in "; break;
        }
        return a.toString() + kindStr + b.toString();
    }
};

// Manages borrowing and lifetime inference
class BorrowChecker {
private:
    SymbolTable& symbolTable;
    TypeRegistry& typeRegistry;
    std::vector<std::string> errors;
    std::vector<LifetimeConstraint> constraints;
    
    // Maps from AST nodes to their associated lifetimes
    std::unordered_map<ExprAST*, Lifetime> exprLifetimes;
    
    // Maps from variables to their active borrows
    struct BorrowInfo {
        Symbol* symbol;
        SourceLocation location;
        bool isMutable;
        Lifetime lifetime;
    };
    std::unordered_map<std::string, std::vector<BorrowInfo>> activeBorrows;
    
    // Helper methods
    void recordBorrow(Symbol* borrowed, Symbol* borrower, bool isMutable, const SourceLocation& loc);
    bool canBorrow(Symbol* symbol, bool mutable_, const SourceLocation& loc);
    void addConstraint(const Lifetime& a, const Lifetime& b, LifetimeConstraint::Kind kind, const SourceLocation& loc);
    bool solveConstraints();
    
public:
    BorrowChecker(SymbolTable& symTable, TypeRegistry& typeReg)
        : symbolTable(symTable), typeRegistry(typeReg) {}
    
    // Main entry point - check a program for borrow safety
    bool checkProgram(Program* program);
    
    // Check a function for borrow safety
    bool checkFunc(FuncAST* func);
    
    // Check an expression for borrow safety
    bool checkExpression(ExprAST* expr, ScopeNode* scope);
    
    // Specific expression type checks
    bool checkVariableExpr(VariableExprAST* expr, ScopeNode* scope);
    bool checkBinaryExpr(BinaryExprAST* expr, ScopeNode* scope);
    bool checkCallExpr(CallExprAST* expr, ScopeNode* scope);
    bool checkBlockExpr(BlockExprAST* expr, ScopeNode* scope);
    
    // Lifetime inference methods
    Lifetime inferLifetime(ExprAST* expr);
    Lifetime getCommonLifetime(const Lifetime& a, const Lifetime& b);
    
    // Access results
    const std::vector<std::string>& getErrors() const { return errors; }
    bool hasErrors() const { return !errors.empty(); }
    void addError(const std::string& msg, const SourceLocation& loc);
    
    // Get the inferred lifetime for an expression
    Lifetime getExprLifetime(ExprAST* expr) {
        auto it = exprLifetimes.find(expr);
        if (it != exprLifetimes.end()) {
            return it->second;
        }
        return Lifetime::createInferred();
    }
    
    // Set the inferred lifetime for an expression
    void setExprLifetime(ExprAST* expr, const Lifetime& lifetime) {
        exprLifetimes[expr] = lifetime;
    }
};

// Implementation of key methods to demonstrate how the system works
void BorrowChecker::addError(const std::string& msg, const SourceLocation& loc) {
    std::string errorMsg = loc.filename + ":" + std::to_string(loc.line) + ":" + 
                           std::to_string(loc.column) + ": error: " + msg;
    errors.push_back(errorMsg);
}

bool BorrowChecker::checkProgram(Program* program) {
    if (!program) return false;
    
    // Check all functions in the program
    for (const auto& func : program->getFunctions()) {
        if (!checkFunc(func.get())) {
            return false;
        }
    }
    
    // Check all top-level statements
    for (const auto& stmt : program->getStatements()) {
        if (!checkExpression(stmt->getExpression(), symbolTable.getRootScope())) {
            return false;
        }
    }
    
    // Solve lifetime constraints
    return solveConstraints();
}

bool BorrowChecker::checkFunc(FuncAST* func) {
    if (!func) return false;
    
    // Enter a new scope for the function
    ScopeNode* functionScope = symbolTable.enterScope(
        func->getLocation().filename,
        func->getLocation().line,
        func->getLocation().column
    );
    
    // Check function body
    bool result = checkExpression(func->getBody(), functionScope);
    
    // Exit function scope
    symbolTable.exitScope(
        func->getBody()->getLocation().line + 1,  // Just an estimate
        func->getBody()->getLocation().column
    );
    
    return result;
}

bool BorrowChecker::checkExpression(ExprAST* expr, ScopeNode* scope) {
    if (!expr) return true;
    
    // Dispatch to appropriate handler based on expression type
    if (auto* varExpr = dynamic_cast<VariableExprAST*>(expr)) {
        return checkVariableExpr(varExpr, scope);
    } else if (auto* binExpr = dynamic_cast<BinaryExprAST*>(expr)) {
        return checkBinaryExpr(binExpr, scope);
    } else if (auto* callExpr = dynamic_cast<CallExprAST*>(expr)) {
        return checkCallExpr(callExpr, scope);
    } else if (auto* blockExpr = dynamic_cast<BlockExprAST*>(expr)) {
        return checkBlockExpr(blockExpr, scope);
    }
    
    // For other expression types, just record the lifetime as static
    setExprLifetime(expr, Lifetime::createStatic());
    return true;
}

bool BorrowChecker::checkVariableExpr(VariableExprAST* expr, ScopeNode* scope) {
    const std::string& varName = expr->getName();
    Symbol* symbol = symbolTable.lookup(varName);
    
    if (!symbol) {
        addError("Use of undeclared variable '" + varName + "'", expr->getLocation());
        return false;
    }
    
    // Record usage of the variable
    symbol->addUsage(expr->getLocation().filename, expr->getLocation().line, 
                    expr->getLocation().column);
    
    // Assign a lifetime based on the variable's scope
    Lifetime lifetime;
    if (symbol->isOwner()) {
        // Owners have lifetimes tied to their scope
        lifetime = Lifetime::createForScope(scope->getId());
    } else if (symbol->isReference()) {
        // References have lifetimes tied to what they point to
        if (symbol->pointsTo.has_value()) {
            Symbol* pointedTo = symbolTable.lookup(symbol->pointsTo.value());
            if (pointedTo) {
                // The reference's lifetime is constrained by the lifetime of what it points to
                lifetime = inferLifetime(expr);  // Will be set in solveConstraints
                
                // Add a constraint that the reference cannot outlive the referent
                Lifetime pointedToLifetime = Lifetime::createForScope(scope->getId());
                addConstraint(lifetime, pointedToLifetime, LifetimeConstraint::Kind::Contained, 
                             expr->getLocation());
            }
        }
    }
    
    setExprLifetime(expr, lifetime);
    return true;
}

Lifetime BorrowChecker::inferLifetime(ExprAST* expr) {
    // A basic implementation that returns a static lifetime for now
    return Lifetime::createStatic();
}

bool BorrowChecker::checkBinaryExpr(BinaryExprAST* expr, ScopeNode* scope) {
    // Check both sides of the expression
    if (!checkExpression(expr->getLHS(), scope) || !checkExpression(expr->getRHS(), scope)) {
        return false;
    }
    
    // For assignment operations, check if we're assigning to a borrowed value
    if (expr->getOperator() == '=') {
        if (auto* varExpr = dynamic_cast<VariableExprAST*>(expr->getLHS())) {
            Symbol* symbol = symbolTable.lookup(varExpr->getName());
            if (symbol && !symbol->isOwner()) {
                // Check if this is a mutable reference
                if (!symbol->isMutBorrowed()) {
                    addError("Cannot assign to immutable borrowed value", expr->getLocation());
                    return false;
                }
            }
        }
    }
    
    // The lifetime of the expression is the shorter of its operands
    Lifetime lhsLifetime = getExprLifetime(expr->getLHS());
    Lifetime rhsLifetime = getExprLifetime(expr->getRHS());
    setExprLifetime(expr, getCommonLifetime(lhsLifetime, rhsLifetime));
    
    return true;
}

bool BorrowChecker::checkCallExpr(CallExprAST* expr, ScopeNode* scope) {
    // Check all arguments
    for (const auto& arg : expr->getArgs()) {
        if (!checkExpression(arg.get(), scope)) {
            return false;
        }
    }
    
    // For now, assume function calls have 'static lifetime
    // This would be refined with proper function signature analysis
    setExprLifetime(expr, Lifetime::createStatic());
    return true;
}

bool BorrowChecker::checkBlockExpr(BlockExprAST* expr, ScopeNode* scope) {
    // Create a new scope for the block
    ScopeNode* blockScope = symbolTable.enterScope(
        expr->getLocation().filename,
        expr->getLocation().line,
        expr->getLocation().column
    );
    
    // Check all expressions in the block
    bool success = true;
    Lifetime blockLifetime = Lifetime::createStatic();
    
    for (const auto& subExpr : expr->getExpressions()) {
        if (!checkExpression(subExpr.get(), blockScope)) {
            success = false;
        }
        
        // Update block's lifetime to be the shortest in the block
        Lifetime exprLifetime = getExprLifetime(subExpr.get());
        blockLifetime = getCommonLifetime(blockLifetime, exprLifetime);
    }
    
    // Exit block scope
    symbolTable.exitScope(
        expr->getLocation().line + expr->getExpressions().size(),  // Estimate
        expr->getLocation().column
    );
    
    // Set the lifetime of the block to the lifetime of its last expression
    // or static if empty
    setExprLifetime(expr, blockLifetime);
    
    return success;
}

Lifetime BorrowChecker::getCommonLifetime(const Lifetime& a, const Lifetime& b) {
    // Static lifetime dominates
    if (a.getKind() == Lifetime::Kind::Static) return a;
    if (b.getKind() == Lifetime::Kind::Static) return b;
    
    // If either is inferred, return the other
    if (a.getKind() == Lifetime::Kind::Inferred) return b;
    if (b.getKind() == Lifetime::Kind::Inferred) return a;
    
    // Otherwise, create a new inferred lifetime with a constraint
    Lifetime result = Lifetime::createInferred();
    
    // Add constraints (to be resolved later)
    addConstraint(result, a, LifetimeConstraint::Kind::Contained, SourceLocation());
    addConstraint(result, b, LifetimeConstraint::Kind::Contained, SourceLocation());
    
    return result;
}

void BorrowChecker::addConstraint(const Lifetime& a, const Lifetime& b, 
                                 LifetimeConstraint::Kind kind, const SourceLocation& loc) {
    constraints.emplace_back(a, b, kind, loc);
}

bool BorrowChecker::solveConstraints() {
    // This is a simplified implementation
    // A real solver would build a graph and check for valid solutions
    
    // For now, just check obvious violations
    for (const auto& constraint : constraints) {
        if (constraint.kind == LifetimeConstraint::Kind::Contained) {
            // Check if a cannot be contained in b
            if (constraint.a.getKind() == Lifetime::Kind::Static && 
                constraint.b.getKind() != Lifetime::Kind::Static) {
                addError("Lifetime constraint violation: " + constraint.toString(), 
                       constraint.location);
                return false;
            }
        }
    }
    
    return true;
}

void BorrowChecker::recordBorrow(Symbol* borrowed, Symbol* borrower, 
                                bool isMutable, const SourceLocation& loc) {
    if (!borrowed || !borrower) return;
    
    // Record that this variable is being borrowed
    borrowed->addReferencer(borrower->name);
    
    // Record the borrow information
    activeBorrows[borrowed->name].push_back({
        borrower,
        loc,
        isMutable,
        Lifetime::createInferred()  // Will be refined during constraint solving
    });
    
    // Set up the reference relationship
    borrower->pointsTo = borrowed->name;
}

bool BorrowChecker::canBorrow(Symbol* symbol, bool mutable_, const SourceLocation& loc) {
    if (!symbol) return false;
    
    // Check active borrows
    const auto& borrows = activeBorrows[symbol->name];
    
    // For mutable borrows, ensure no other borrows exist
    if (mutable_) {
        if (!borrows.empty()) {
            addError("Cannot mutably borrow '" + symbol->name + "' as it is already borrowed", loc);
            return false;
        }
    } else {
        // For immutable borrows, ensure no mutable borrows exist
        for (const auto& borrow : borrows) {
            if (borrow.isMutable) {
                addError("Cannot immutably borrow '" + symbol->name + "' as it is already mutably borrowed", loc);
                return false;
            }
        }
    }
    
    return true;
}

#endif // __BORROW_CHECKER_H__