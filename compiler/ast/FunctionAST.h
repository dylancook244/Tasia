#ifndef __FUNCTION_AST_H__
#define __FUNCTION_AST_H__

#include "ASTNode.h"
#include "PrototypeAST.h"
#include "ExprAST.h"
#include <memory>

class FunctionAST : public ASTNode {
private:
    std::unique_ptr<PrototypeAST> proto;
    std::unique_ptr<ExprAST> body;
    
public:
    FunctionAST(std::unique_ptr<PrototypeAST> proto,
               std::unique_ptr<ExprAST> body)
        : proto(std::move(proto)), body(std::move(body)) {}
    
    PrototypeAST *getPrototype() const { return proto.get(); }
    ExprAST *getBody() const { return body.get(); }
    
    virtual std::string toString() const override {
        return "Function(" + 
               (proto ? proto->toString() : "null") + ", " +
               (body ? body->toString() : "null") + ")";
    }
};

#endif // __FUNCTION_AST_H__