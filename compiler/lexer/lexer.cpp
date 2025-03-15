#include "lexer/token.h"
#include "lexer/lexer.h"
#include <string>
#include <cctype>
#include <iostream>

Lexer::Lexer(const std::string &source) 
    : sourceBuffer(source),
      currentPosition(0),
      identifierStr(""),
      numVal(0.0),
      lastChar(' ')
{}

// basically increments unless it's the end of the file, then stop
char Lexer::getNextChar() {
    // return if at end
    if (currentPosition >= sourceBuffer.length()) {
        return END_OF_FILE;
    }
    // otherwise advance one space
    return sourceBuffer[currentPosition++];
}

int Lexer::getNextToken() {
    while (true) {
        // skip white space
        while (isspace(lastChar)) {
            lastChar = getNextChar();
        }

        // handles keywords and identifiers
        if (isalpha(lastChar) || lastChar == '_') {
            identifierStr = lastChar;
            while (isalnum(lastChar = getNextChar()) || lastChar == '_') {
                identifierStr += lastChar;
            }

            // keyword checking
            if (identifierStr == "func") return FUNC;
            return IDENT;
        }

        // handle ints
        if (isdigit(lastChar) || lastChar == '.') {
            std::string numStr;

            do {
                numStr += lastChar;
                lastChar = getNextChar();
            } while (isdigit(lastChar) || lastChar == '.');

            numVal = strtod(numStr.c_str(), nullptr);
            return INT;
        }

        // new line chars
        if (lastChar == '\n') {
            lastChar = getNextChar();
            return LINE_END;
        }

        // handles continuations
        if (lastChar == '\\') {
            lastChar = getNextChar();

            if (lastChar == '\n') {
                lastChar = getNextChar();
                continue;
            } 
            return '\\';
        } 

        // handles comments
        if (lastChar == '#') {
            do {
                lastChar = getNextChar();
            } while (lastChar != '\n' && lastChar != '\r' && lastChar != END_OF_FILE);

            if (lastChar != END_OF_FILE) {
                lastChar = getNextChar(); // get rid of the newline
                continue;
            }
        }

        if (lastChar == END_OF_FILE) {
            return END_OF_FILE;
        }

        if (lastChar == '{' || lastChar == '}' || lastChar == '(' || lastChar == ')') {
            int thisChar = lastChar;
            lastChar = getNextChar();
            return thisChar;
        }

        if (lastChar == '+' || lastChar == '-' || lastChar == '*' || lastChar == '/') {
            int thisChar = lastChar;
            lastChar = getNextChar();
            return thisChar;
        }

        
        if (lastChar == '=') {
            int thisChar = lastChar;
            lastChar = getNextChar();
            return thisChar;
        }
        

        std::cerr << "Unhandled token: '" 
          << static_cast<char>(lastChar)  // Convert number to character
          << "' (ASCII " << static_cast<int>(lastChar) << ")"
          << std::endl;

        exit(1);
        
    }
}