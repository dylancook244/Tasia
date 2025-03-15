#ifndef __FUNCTION_AST_H__
#define __FUNCTION_AST_H__

#include "ASTNode.h"
#include "FuncInterfaceAST.h"
#include "ExprAST.h"
#include <memory>

class FuncAST : public ASTNode {
private:
    std::unique_ptr<FuncInterfaceAST> funcInterface;
    std::unique_ptr<ExprAST> body;
    
public:
    FuncAST(std::unique_ptr<FuncInterfaceAST> funcInterface,
               std::unique_ptr<ExprAST> body)
        : funcInterface(std::move(funcInterface)), body(std::move(body)) {}
    
    FuncInterfaceAST *getFuncInterface() const { return funcInterface.get(); }
    ExprAST *getBody() const { return body.get(); }
    
    virtual std::string toString() const override {
        return "Function(" + 
               (funcInterface ? funcInterface->toString() : "null") + ", " +
               (body ? body->toString() : "null") + ")";
    }
};

#endif // __FUNCTION_AST_H__