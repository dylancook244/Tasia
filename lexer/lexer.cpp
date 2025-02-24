#include "lexer/token.h"
#include "lexer/lexer.h"
#include <string>
#include <cctype>

Lexer::Lexer(const std::string& source) 
    : sourceBuffer(source),
      currentPosition(0),
      identifierStr(""),
      numVal(0.0),
      lastChar(' ')
{}

// basically increments us unless it's the end of the file, then stop us
char Lexer::getNextChar() {
    //return if at end
    if (currentPosition >= sourceBuffer.length()) {
        return EOF;
    }
    // otherwise advance one space
    return sourceBuffer[currentPosition++];
}

int Lexer::getNextToken() {

    // skip white space
    while (isspace(lastChar)) {
        lastChar = getNextChar();
    }

    // handles keywords and identifiers
    if (isalpha(lastChar)) {
        identifierStr = lastChar;
        while (isalnum(lastChar = getNextChar())) {
            identifierStr += lastChar;
        }

        // keyword checking
        if (identifierStr == "func") return tok_func;
        if (identifierStr == "extern") return tok_extern;
        return tok_identifier;
    }

    // handle numbers
    if (isdigit(lastChar) || lastChar == '.') {
        std::string numStr;

        do {
            numStr += lastChar;
            lastChar = getNextChar();
        } while (isdigit(lastChar) || lastChar == '.');

        numVal = strtod(numStr.c_str(), nullptr);
        return tok_number;
    }

    // new line chars
    if (lastChar == '\n') {
        lastChar = getNextChar();
        return tok_line_end;
    }

    // handles continuations
    if (lastChar == '\\') {
        lastChar = getNextChar();

        if (lastChar == '\n') {
            lastChar = getNextChar();
            return getNextToken(); // recursively call itself since it's a continuation
        } 
        return '\\';
    } 

    // handles comments
    if (lastChar == '#') {
        do {
            lastChar = getNextChar();
        } while (lastChar != '\n' && lastChar != '\r' && lastChar != EOF);

        if (lastChar != EOF) {
            return getNextToken();
        }
    }

    if (lastChar == EOF) {
        return tok_eof;
    }

    // if we have any other single char tokens
    int thisChar = lastChar;
    lastChar = getNextChar();
    return thisChar;
}