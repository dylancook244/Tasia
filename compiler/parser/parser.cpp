#include "parser/parser.h"

// Constructor initializes the parser
Parser::Parser(Lexer& lex, const std::string& filename) 
    : lexer(lex), 
      currentToken(lex.getNextToken()), // <- first getNextToken() call in compiler
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
void Parser::addError(const std::string& msg) {
    // In a more advanced implementation, we would get line/column from lexer
    errors.push_back({msg, {currentFilename, 0, 0}});
}

std::unique_ptr<ExprAST> Parser::logError(const char* str) {
    addError(str);
    return nullptr;
}

std::unique_ptr<FuncInterfaceAST> Parser::logErrorP(const char* str) {
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

// This routine expects to be called when the current token is a INT
std::unique_ptr<ExprAST> Parser::parseNumberExpr() {
    auto Result = std::make_unique<NumberExprAST>(lexer.getNumber());
    getNextToken();
    return std::move(Result);
}

// This routine parses expressions in "(" and ")" characters
std::unique_ptr<ExprAST> Parser::parseParenExpr() {
    getNextToken(); // drop '('

    auto V = parseExpression();
    if (!V) {
        return nullptr;
    }

    if (currentToken != ')') {
        return logError("Expected ')'");
    }

    getNextToken(); // drop ')'
    return V;
}

// This routine expects to be called when current token is IDENT
std::unique_ptr<ExprAST> Parser::parseIdentifierExpr() {
    std::string idName = lexer.getIdentifier();

    getNextToken(); // drop identifier

    if (currentToken != '(') {
        return std::make_unique<VariableExprAST>(idName);
    }

    getNextToken(); // drop '('
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

            getNextToken(); // drop ','
        }
    }

    getNextToken(); // drop ')'

    return std::make_unique<CallExprAST>(idName, std::move(args));
}

std::unique_ptr<ExprAST> Parser::parsePrimary() {
    switch (currentToken) {
        default:
            if (currentToken == '}') {
                // We found a closing brace outside a block - could be an error
                // But we need to consume it and return something to break infinite loop
                getNextToken(); // consume the '}'
                return logError("Unexpected closing brace '}'");
            }
            return logError("Unknown token when expecting an expression");
        case IDENT:
            return parseIdentifierExpr();
        case '(':
            return parseParenExpr();
        case '{':
            return parseBlockExpr();
        case '=':
            return parseDeclaration();
        case '&':
            return parseReference();
        case '*':
            return parseDereference();
    }
}

std::unique_ptr<ExprAST> Parser::parseBinOpRHS(int exprPrec, std::unique_ptr<ExprAST> LHS) {
    while (true) {
        int tokPrec = getTokPrecedence();

        if (tokPrec < exprPrec) {
            return LHS;
        }

        int binOp = currentToken;
        getNextToken(); // drop the operator

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

std::unique_ptr<ExprAST> Parser::parseBlockExpr() {
    getNextToken(); // drop '{'
    
    std::vector<std::unique_ptr<ExprAST>> expressions;
    
    // Parse expressions until we hit closing brace
    while (currentToken != '}' && currentToken != END_OF_FILE) {
        // Skip empty lines
        if (currentToken == LINE_END) {
            getNextToken();
            continue;
        }
        
        auto expr = parseExpression();
        if (!expr) {
            return nullptr;
        }
        
        // Allow but don't require line ends between expressions
        // Just add the expression to our list
        expressions.push_back(std::move(expr));
        
        // If we see a line end, consume it
        if (currentToken == LINE_END) {
            getNextToken();
        }
    }
    
    if (currentToken != '}') {
        return logError("Expected '}' at end of block");
    }
    getNextToken(); // drop '}'
    
    return std::make_unique<BlockExprAST>(std::move(expressions));
}


std::unique_ptr<FuncInterfaceAST> Parser::parseFuncInterface() {
    if (currentToken != IDENT) {
        return logErrorP("Expected function name in function interface");
    }

    std::string funcName = lexer.getIdentifier();

    // rename the entry point so we can use main() in tasia
    if (funcName == "main") {
        funcName = "TASIA_ENTRY_FUNCTION";
    }

    getNextToken(); // drop identifier name

    if (currentToken != '(') {
        return logErrorP("Expected '(' in function interface");
    }

    std::vector<std::string> argNames;
    while (getNextToken() == IDENT) {
        argNames.push_back(lexer.getIdentifier());
    }

    if (currentToken != ')') {
        return logErrorP("Expected ')' in function interface");
    }

    getNextToken(); // drop ')'

    return std::make_unique<FuncInterfaceAST>(funcName, std::move(argNames));
}

std::unique_ptr<FuncAST> Parser::parseDefinition() {
    getNextToken(); // drop 'func'
    
    auto funcInterface = parseFuncInterface();
    if (!funcInterface) {
        return nullptr;
    }

    // Require a block for function body
    if (currentToken != '{') {
        logError("Expected '{' to begin function body");
        return nullptr;
    }
    
    // Parse the block
    auto blockExpr = parseBlockExpr();
    if (!blockExpr) {
        return nullptr;
    }
    
    return std::make_unique<FuncAST>(std::move(funcInterface), std::move(blockExpr));
}

std::unique_ptr<FuncInterfaceAST> Parser::parseExtern() {
    getNextToken(); // drop 'extern'
    return parseFuncInterface();
}

std::unique_ptr<StmtAST> Parser::parseStatement() {
    auto expr = parseExpression();
    if (!expr) return nullptr;

    // need line end
    if (currentToken != LINE_END) {
        logError("Expected end of line");
        return nullptr;
    }

    getNextToken(); // drop newline
    return std::make_unique<StmtAST>(std::move(expr));
}

std::unique_ptr<ExprAST> Parser::parseDeclaration() {
    // Expect 'let' keyword
    getNextToken(); // drop 'let'
    
    if (currentToken != IDENT) {
        return logError("Expected identifier after 'let'");
    }
    
    std::string name = lexer.getIdentifier();
    getNextToken(); // drop identifier
    
    // Check for mutability specifier
    bool mutable_ = true;
    if (currentToken == MUT) {
        mutable_ = true;
        getNextToken(); // drop 'mut'
    } else if (currentToken == CONST) {
        mutable_ = false;
        getNextToken(); // drop 'const'
    }
    
    // Check for type annotation
    std::string typeHint;
    if (currentToken == ':') {
        getNextToken(); // drop ':'
        
        if (currentToken != IDENT) {
            return logError("Expected type name after ':'");
        }
        
        typeHint = lexer.getIdentifier();
        getNextToken(); // drop type name
    }
    
    // Check for initialization
    std::unique_ptr<ExprAST> initExpr;
    if (currentToken == '=') {
        getNextToken(); // drop '='
        
        initExpr = parseExpression();
        if (!initExpr) {
            return nullptr;
        }
    }
    
    return std::make_unique<DeclarationExprAST>(name, std::move(initExpr), typeHint, mutable_);
}

std::unique_ptr<ExprAST> Parser::parseReference() {
    // Expect '&' character
    getNextToken(); // drop '&'
    
    // Check if it's a mutable reference
    bool mutable_ = false;
    if (currentToken == MUT) {
        mutable_ = true;
        getNextToken(); // drop 'mut'
    }
    
    auto target = parsePrimary();
    if (!target) {
        return nullptr;
    }
    
    return std::make_unique<ReferenceExprAST>(std::move(target), mutable_);
}

std::unique_ptr<ExprAST> Parser::parseDereference() {
    // Expect '*' character
    getNextToken(); // drop '*'
    
    auto target = parsePrimary();
    if (!target) {
        return nullptr;
    }
    
    return std::make_unique<DereferenceExprAST>(std::move(target));
}

// Main parsing entry point
std::unique_ptr<Program> Parser::parseFile() {
    auto program = std::make_unique<Program>(currentFilename);

    while (currentToken != END_OF_FILE) {
        switch (currentToken) {
            case FUNC: {
                auto func = parseDefinition();
                if (func)
                    program->addFunc(std::move(func));
                break;
            }
            case LINE_END:
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