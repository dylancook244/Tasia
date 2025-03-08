#ifndef __NUMBER_EXPR_AST_H__
#define __NUMBER_EXPR_AST_H__

#include "ExprAST.h"
#include <sstream>

class NumberExprAST : public ExprAST {
private:
    double val;
    
public:
    NumberExprAST(double val) : val(val) {}
    
    double getValue() const { return val; }
    
    virtual std::string toString() const override {
        std::stringstream ss;
        ss << "NumberExpr(" << val << ")";
        return ss.str();
    }
};

#endif // __NUMBER_EXPR_AST_H__