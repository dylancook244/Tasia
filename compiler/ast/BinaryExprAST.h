#ifndef __BINARY_EXPR_AST_H__
#define __BINARY_EXPR_AST_H__

#include "ExprAST.h"
#include <memory>
#include <string>

class BinaryExprAST : public ExprAST {
private:
    char op;
    std::unique_ptr<ExprAST> lhs, rhs;
    
public:
    BinaryExprAST(char op, std::unique_ptr<ExprAST> lhs,
                 std::unique_ptr<ExprAST> rhs)
        : op(op), lhs(std::move(lhs)), rhs(std::move(rhs)) {}
    
    char getOperator() const { return op; }
    ExprAST *getLHS() const { return lhs.get(); }
    ExprAST *getRHS() const { return rhs.get(); }
    
    virtual std::string toString() const override {
        std::string opStr;
        switch (op) {
            case '+': opStr = "add"; break;
            case '-': opStr = "sub"; break;
            case '*': opStr = "mul"; break;
            case '/': opStr = "div"; break;
            default: opStr = std::string(1, op);
        }
        
        return "BinExpr(" + opStr + ", " + 
               (lhs ? lhs->toString() : "null") + ", " + 
               (rhs ? rhs->toString() : "null") + ")";
    }
};

#endif // __BINARY_EXPR_AST_H__