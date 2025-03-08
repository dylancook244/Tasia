// Program.h
#ifndef __PROGRAM_H__
#define __PROGRAM_H__

#include "FunctionAST.h"
#include "StmtAST.h"
#include <vector>
#include <memory>
#include <string>

class Program : public ASTNode {
private:
    std::vector<std::unique_ptr<FunctionAST>> functions;
    std::vector<std::unique_ptr<StmtAST>> statements;
    std::string sourceFile;
    
public:
    Program(const std::string &source = "") : sourceFile(source) {}
    
    void addFunction(std::unique_ptr<FunctionAST> func) {
        functions.push_back(std::move(func));
    }
    
    void addStatement(std::unique_ptr<StmtAST> stmt) {
        statements.push_back(std::move(stmt));
    }
    
    const std::vector<std::unique_ptr<FunctionAST>> &getFunctions() const {
        return functions;
    }
    
    const std::vector<std::unique_ptr<StmtAST>> &getStatements() const {
        return statements;
    }
    
    const std::string &getSourceFile() const {
        return sourceFile;
    }
    
    virtual std::string toString() const override {
        std::string result = "Program(" + sourceFile + ",\n  Functions: [";
        for (size_t i = 0; i < functions.size(); ++i) {
            if (i > 0) result += ",\n    ";
            result += functions[i] ? functions[i]->toString() : "null";
        }
        result += "],\n  Statements: [";
        for (size_t i = 0; i < statements.size(); ++i) {
            if (i > 0) result += ",\n    ";
            result += statements[i] ? statements[i]->toString() : "null";
        }
        result += "]\n)";
        return result;
    }
};

#endif // __PROGRAM_H__