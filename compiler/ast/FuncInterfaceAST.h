#ifndef __FUNC_INTERFACE_AST_H__
#define __FUNC_INTERFACE_AST_H__

#include "ASTNode.h"
#include <string>
#include <vector>

class FuncInterfaceAST : public ASTNode {
private:
    std::string name;
    std::vector<std::string> args;
    
public:
    FuncInterfaceAST(const std::string &name, std::vector<std::string> args)
        : name(name), args(std::move(args)) {}
    
    const std::string &getName() const { return name; }
    const std::vector<std::string> &getArgs() const { return args; }
    
    virtual std::string toString() const override {
        std::string result = "FuncInterface(" + name + ", [";
        for (size_t i = 0; i < args.size(); ++i) {
            if (i > 0) result += ", ";
            result += args[i];
        }
        result += "])";
        return result;
    }
};

#endif // __FUNC_INTERFACE_AST_H__