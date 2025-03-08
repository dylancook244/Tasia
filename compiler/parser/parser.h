#ifndef __PARSER_H__
#define __PARSER_H__

#include <map>
#include <vector>
#include <string>
#include <memory>
#include <iostream>
#include "lexer/lexer.h"
#include "lexer/token.h"
#include "ast/ExprAST.h"
#include "ast/NumberExprAST.h"
#include "ast/VariableExprAST.h"
#include "ast/BinaryExprAST.h"
#include "ast/CallExprAST.h"
#include "ast/PrototypeAST.h"
#include "ast/FunctionAST.h"
#include "ast/BlockExprAST.h"
#include "ast/StmtAST.h"
#include "ast/Program.h"

// Error structure with source location
struct ParseError {
    std::string message;
    SourceLocation location;
};

class Parser {
private:
    Lexer &lexer;
    int currentToken;
    std::map<char, int> binopPrecedence;
    std::vector<ParseError> errors;
    std::string currentFilename;
    int currentLine;
    int currentColumn;
    
    // Helper methods
    int getNextToken();
    int getTokPrecedence();
    SourceLocation getCurrentLocation();
    void addError(const std::string &msg);
    
    // Parsing methods
    std::unique_ptr<ExprAST> parseNumberExpr();
    std::unique_ptr<ExprAST> parseParenExpr();
    std::unique_ptr<ExprAST> parseIdentifierExpr();
    std::unique_ptr<ExprAST> parsePrimary();
    std::unique_ptr<ExprAST> parseBinOpRHS(int exprPrec, std::unique_ptr<ExprAST> LHS);
    std::unique_ptr<ExprAST> parseExpression();
    std::unique_ptr<ExprAST> parseBlockExpr();
    std::unique_ptr<PrototypeAST> parsePrototype();
    std::unique_ptr<FunctionAST> parseDefinition();
    std::unique_ptr<PrototypeAST> parseExtern();
    std::unique_ptr<StmtAST> parseStatement();
    
    // Error handling helpers
    std::unique_ptr<ExprAST> logError(const char *str);
    std::unique_ptr<PrototypeAST> logErrorP(const char *str);
    
public:
    // Constructor sets up the parser with a lexer and initializes operator precedence
    Parser(Lexer &lex, const std::string &filename = "<stdin>");
    
    // Main parsing entry point
    std::unique_ptr<Program> parseFile();
    
    // Access parsing results
    const std::vector<ParseError> &getErrors() const { return errors; }
    bool hasErrors() const { return !errors.empty(); }
};

#endif // __PARSER_H__