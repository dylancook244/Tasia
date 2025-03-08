#ifndef __BLOCK_EXPR_AST_H__
#define __BLOCK_EXPR_AST_H__

#include "ExprAST.h"
#include <vector>
#include <memory>

class BlockExprAST : public ExprAST {
private:
    std::vector<std::unique_ptr<ExprAST>> expressions;
    
public:
    BlockExprAST(std::vector<std::unique_ptr<ExprAST>> expressions)
        : expressions(std::move(expressions)) {}
    
    const std::vector<std::unique_ptr<ExprAST>> &getExpressions() const { 
        return expressions; 
    }
    
    virtual std::string toString() const override {
        std::string result = "BlockExpr([";
        for (size_t i = 0; i < expressions.size(); ++i) {
            if (i > 0) result += ", ";
            result += expressions[i] ? expressions[i]->toString() : "null";
        }
        result += "])";
        return result;
    }
};

#endif // __BLOCK_EXPR_AST_H__