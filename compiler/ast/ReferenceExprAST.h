#ifndef __REFERENCE_EXPR_AST_H__
#define __REFERENCE_EXPR_AST_H__

#include "ExprAST.h"
#include <memory>

class ReferenceExprAST : public ExprAST {
private:
    std::unique_ptr<ExprAST> target;
    bool mutable_;
    
public:
    ReferenceExprAST(std::unique_ptr<ExprAST> target, bool mutable_ = false)
        : target(std::move(target)), mutable_(mutable_) {}
    
    ExprAST* getTarget() const { return target.get(); }
    bool isMutable() const { return mutable_; }
    
    virtual std::string toString() const override {
        std::string result = "RefExpr(";
        if (mutable_) result += "mut ";
        result += target ? target->toString() : "null";
        result += ")";
        return result;
    }
};

#endif // __REFERENCE_EXPR_AST_H__