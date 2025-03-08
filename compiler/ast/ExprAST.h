#ifndef __EXPR_AST_H__
#define __EXPR_AST_H__

#include "ASTNode.h"

// Base class for all expression nodes
class ExprAST : public ASTNode {
public:
    ExprAST() = default;
    virtual ~ExprAST() = default;
};

#endif // __EXPR_AST_H__