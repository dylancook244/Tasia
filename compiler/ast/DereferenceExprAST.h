#ifndef __DEREFERENCE_EXPR_AST_H__
#define __DEREFERENCE_EXPR_AST_H__

#include "ExprAST.h"
#include <memory>

class DereferenceExprAST : public ExprAST {
private:
    std::unique_ptr<ExprAST> target;
    
public:
    DereferenceExprAST(std::unique_ptr<ExprAST> target)
        : target(std::move(target)) {}
    
    ExprAST* getTarget() const { return target.get(); }
    
    virtual std::string toString() const override {
        return "DerefExpr(" + (target ? target->toString() : "null") + ")";
    }
};

#endif // __DEREFERENCE_EXPR_AST_H__