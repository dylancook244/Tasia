#ifndef __VARIABLE_EXPR_AST_H__
#define __VARIABLE_EXPR_AST_H__

#include "ExprAST.h"
#include <string>

class VariableExprAST : public ExprAST {
private:
    std::string name;
    bool isDeclaration = false;  // Is this a variable declaration?
    
public:
    VariableExprAST(const std::string& name, bool isDecl = false) 
        : name(name), isDeclaration(isDecl) {}
    
    const std::string& getName() const { return name; }
    bool getIsDeclaration() const { return isDeclaration; }
    void setIsDeclaration(bool isDecl) { isDeclaration = isDecl; }
    
    virtual std::string toString() const override {
        std::string result = isDeclaration ? "VarDecl(" : "VarExpr(";
        result += name;
        
        if (getPointsTo().has_value()) {
            result += ", points_to=" + getPointsTo().value();
        }
        
        if (getIsOwner()) {
            result += ", owner";
        } else {
            result += ", reference";
        }
        
        if (!getIsMutable()) {
            result += ", immutable";
        }
        
        result += ")";
        return result;
    }
};

#endif // __VARIABLE_EXPR_AST_H__