#ifndef __VARIABLE_EXPR_AST_H__
#define __VARIABLE_EXPR_AST_H__

#include "ExprAST.h"
#include <string>

class VariableExprAST : public ExprAST {
private:
    std::string name;
    
public:
    VariableExprAST(const std::string& name) : name(name) {}
    
    const std::string &getName() const { return name; }
    
    virtual std::string toString() const override {
        return "VarExpr(" + name + ")";
    }
};

#endif // __VARIABLE_EXPR_AST_H__