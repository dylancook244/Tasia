#include "parser/parser.h"

// Constructor initializes the parser
Parser::Parser(Lexer &lex, const std::string &filename) 
    : lexer(lex), 
      currentToken(lex.getNextToken()),
      currentFilename(filename) 
{
    // Initialize operator precedence
    binopPrecedence['<'] = 10;
    binopPrecedence['+'] = 20;
    binopPrecedence['-'] = 20;
    binopPrecedence['*'] = 40;
    binopPrecedence['/'] = 40;
}

// Get the next token from the lexer
int Parser::getNextToken() {
    currentToken = lexer.getNextToken();
    return currentToken;
}

// Error handling methods
void Parser::addError(const std::string &msg) {
    // In a more advanced implementation, we would get line/column from lexer
    errors.push_back({msg, {currentFilename, 0, 0}});
}

std::unique_ptr<ExprAST> Parser::logError(const char *str) {
    addError(str);
    return nullptr;
}

std::unique_ptr<PrototypeAST> Parser::logErrorP(const char *str) {
    addError(str);
    return nullptr;
}

// Get operator precedence
int Parser::getTokPrecedence() {
    if (!isascii(currentToken)) {
        return -1;
    }

    int tokPrec = binopPrecedence[currentToken];
    if (tokPrec <= 0) return -1;

    return tokPrec;
}

// This routine expects to be called when the current token is a tok_number
std::unique_ptr<ExprAST> Parser::parseNumberExpr() {
    auto Result = std::make_unique<NumberExprAST>(lexer.getNumber());
    getNextToken();
    return std::move(Result);
}

// This routine parses expressions in "(" and ")" characters
std::unique_ptr<ExprAST> Parser::parseParenExpr() {
    getNextToken(); // eat '('

    auto V = parseExpression();
    if (!V) {
        return nullptr;
    }

    if (currentToken != ')') {
        return logError("Expected ')'");
    }

    getNextToken(); // eat ')'
    return V;
}

// This routine expects to be called when current token is tok_identifier
std::unique_ptr<ExprAST> Parser::parseIdentifierExpr() {
    std::string idName = lexer.getIdentifier();

    getNextToken(); // eat identifier

    if (currentToken != '(') {
        return std::make_unique<VariableExprAST>(idName);
    }

    getNextToken(); // eat '('
    std::vector<std::unique_ptr<ExprAST>> args;
    if (currentToken != ')') {
        while (true) {
            if (auto arg = parseExpression()) {
                args.push_back(std::move(arg));
            } else {
                return nullptr;
            }

            if (currentToken == ')') {
                break;
            }

            if (currentToken != ',') {
                return logError("Expected ')' or ',' in argument list");
            }

            getNextToken(); // eat ','
        }
    }

    getNextToken(); // eat ')'

    return std::make_unique<CallExprAST>(idName, std::move(args));
}

std::unique_ptr<ExprAST> Parser::parsePrimary() {
    switch (currentToken) {
        default:
            return logError("Unknown token when expecting an expression");
        case tok_identifier:
            return parseIdentifierExpr();
        case tok_number:
            return parseNumberExpr();
        case '(':
            return parseParenExpr();
    }
}

std::unique_ptr<ExprAST> Parser::parseBinOpRHS(int exprPrec, std::unique_ptr<ExprAST> LHS) {
    while (true) {
        int tokPrec = getTokPrecedence();

        if (tokPrec < exprPrec) {
            return LHS;
        }

        int binOp = currentToken;
        getNextToken(); // eat the operator

        auto RHS = parsePrimary();
        if (!RHS) {
            return nullptr;
        }

        int nextPrec = getTokPrecedence();
        if (tokPrec < nextPrec) {
            RHS = parseBinOpRHS(tokPrec + 1, std::move(RHS));
            if (!RHS) {
                return nullptr;
            }
        }

        LHS = std::make_unique<BinaryExprAST>(binOp, std::move(LHS), std::move(RHS));
    }
}

std::unique_ptr<ExprAST> Parser::parseExpression() {
    auto LHS = parsePrimary();

    if (!LHS) {
        return nullptr;
    }

    return parseBinOpRHS(0, std::move(LHS));
}

std::unique_ptr<PrototypeAST> Parser::parsePrototype() {
    if (currentToken != tok_identifier) {
        return logErrorP("Expected function name in prototype");
    }

    std::string fnName = lexer.getIdentifier();
    getNextToken(); // eat function name

    if (currentToken != '(') {
        return logErrorP("Expected '(' in prototype");
    }

    std::vector<std::string> argNames;
    while (getNextToken() == tok_identifier) {
        argNames.push_back(lexer.getIdentifier());
    }

    if (currentToken != ')') {
        return logErrorP("Expected ')' in prototype");
    }

    getNextToken(); // eat ')'

    return std::make_unique<PrototypeAST>(fnName, std::move(argNames));
}

std::unique_ptr<FunctionAST> Parser::parseDefinition() {
    getNextToken(); // eat 'func'
    
    auto proto = parsePrototype();
    if (!proto) {
        return nullptr;
    }

    if (auto expr = parseExpression()) {
        return std::make_unique<FunctionAST>(std::move(proto), std::move(expr));
    }

    return nullptr;
}

std::unique_ptr<PrototypeAST> Parser::parseExtern() {
    getNextToken(); // eat 'extern'
    return parsePrototype();
}

std::unique_ptr<StmtAST> Parser::parseStatement() {
    auto expr = parseExpression();
    if (!expr) return nullptr;

    // need line end
    if (currentToken != tok_line_end) {
        logError("Expected end of line");
        return nullptr;
    }

    getNextToken(); // eat newline
    return std::make_unique<StmtAST>(std::move(expr));
}

// Main parsing entry point
std::unique_ptr<Program> Parser::parseFile() {
  auto program = std::make_unique<Program>(currentFilename);

  while (currentToken != tok_eof) {
      switch (currentToken) {
          case tok_func: {
              auto func = parseDefinition();
              if (func)
                  program->addFunction(std::move(func));
              break;
          }
          case tok_line_end:
              getNextToken(); // Skip empty lines
              break;
          default: {
              auto stmt = parseStatement();
              if (stmt)
                  program->addStatement(std::move(stmt));
              break;
          }
      }
  }
  return program;
}