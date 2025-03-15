#ifndef __DECLARATION_EXPR_AST_H__
#define __DECLARATION_EXPR_AST_H__

#include "ExprAST.h"
#include <memory>
#include <string>

class DeclarationExprAST : public ExprAST {
private:
    std::string name;
    std::unique_ptr<ExprAST> initExpr;
    std::string typeHint;
    bool mutable_;
    
public:
    DeclarationExprAST(const std::string& name, 
                     std::unique_ptr<ExprAST> init = nullptr,
                     const std::string& typeHint = "",
                     bool mutable_ = true)
        : name(name), initExpr(std::move(init)), 
          typeHint(typeHint), mutable_(mutable_) {}
    
    const std::string& getName() const { return name; }
    ExprAST* getInitExpr() const { return initExpr.get(); }
    const std::string& getTypeHint() const { return typeHint; }
    bool isMutable() const { return mutable_; }
    
    virtual std::string toString() const override {
        std::string result = "Decl(" + name;
        
        if (!typeHint.empty()) {
            result += ": " + typeHint;
        }
        
        if (!mutable_) {
            result += ", immutable";
        }
        
        if (initExpr) {
            result += " = " + initExpr->toString();
        }
        
        result += ")";
        return result;
    }
};

#endif // __DECLARATION_EXPR_AST_H__