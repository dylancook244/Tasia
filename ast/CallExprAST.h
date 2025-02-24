#ifndef __CALL_EXPR_AST_H__
#define __CALL_EXPR_AST_H__

#include "ExprAST.h"
#include <string>
#include <vector>
#include <memory>

class CallExprAST : public ExprAST {
private:
    std::string callee;
    std::vector<std::unique_ptr<ExprAST>> args;
    
public:
    CallExprAST(const std::string& callee,
               std::vector<std::unique_ptr<ExprAST>> args)
        : callee(callee), args(std::move(args)) {}
    
    const std::string& getCallee() const { return callee; }
    const std::vector<std::unique_ptr<ExprAST>>& getArgs() const { return args; }
    
    virtual std::string toString() const override {
        std::string result = "CallExpr(" + callee + ", [";
        for (size_t i = 0; i < args.size(); ++i) {
            if (i > 0) result += ", ";
            result += args[i] ? args[i]->toString() : "null";
        }
        result += "])";
        return result;
    }
};

#endif // __CALL_EXPR_AST_H__