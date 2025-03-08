#ifndef LEXER_H
#define LEXER_H

#include "lexer/token.h"
#include <string>

class Lexer {
    private: 
        // source code we're processing and position in that processing
        std::string sourceBuffer;
        size_t currentPosition;

        std::string identifierStr;
        double numVal;
        int lastChar;

        char getNextChar();

    public:
        // constructor
        Lexer(const std::string& source);

        int getNextToken();

        std::string getIdentifier() const { return identifierStr; }
        double getNumber() const { return numVal; }

};

#endif // LEXER_H