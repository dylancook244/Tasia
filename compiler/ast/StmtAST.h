#ifndef __STMT_AST_H__
#define __STMT_AST_H__

#include "ASTNode.h"
#include "ExprAST.h"
#include <memory>

class StmtAST : public ASTNode {
private:
    std::unique_ptr<ExprAST> expr;
    
public:
    StmtAST(std::unique_ptr<ExprAST> expr) : expr(std::move(expr)) {}
    
    ExprAST *getExpression() const { return expr.get(); }
    
    virtual std::string toString() const override {
        return "Stmt(" + (expr ? expr->toString() : "null") + ")";
    }
};

#endif // __STMT_AST_H__