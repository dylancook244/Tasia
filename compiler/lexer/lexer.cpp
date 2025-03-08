#include "lexer/token.h"
#include "lexer/lexer.h"
#include <string>
#include <cctype>

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
        return EOF;
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
                continue;
            } 
            return '\\';
        } 

        // handles comments
        if (lastChar == '#') {
            do {
                lastChar = getNextChar();
            } while (lastChar != '\n' && lastChar != '\r' && lastChar != EOF);

            if (lastChar != EOF) {
                lastChar = getNextChar(); // get rid of the newline
                continue;
            }
        }

        if (lastChar == EOF) {
            return tok_eof;
        }

        if (lastChar == '{' || lastChar == '}') {
            int thisChar = lastChar;
            lastChar = getNextChar();
            return thisChar;
        }

        // if we have any other single char tokens
        int thisChar = lastChar;
        lastChar = getNextChar();
        return thisChar;
    }
}