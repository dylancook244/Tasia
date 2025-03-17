#ifndef TOKEN_H
#define TOKEN_H

#include <stdbool.h>

// token name in program along with user syntax
typedef enum {

    ILLEGAL = 0,    // N/A
    END_OF_FILE,    // EOF
    END_OF_LINE,    // \n
    COMMENT,        // #

    IDENT,          // user specified

    INT,            // int
    FLOAT,          // float
    CHAR,           // char
    STRING,         // string
    BOOL,           // bool

    INT_LITERAL,            // literal value
    FLOAT_LITERAL,          // literal value
    CHAR_LITERAL,           // literal value
    STRING_LITERAL,         // literal value
    BOOL_LITERAL,           // literal value

    ARRAY,           // literal value
    LIST,           // literal value
    TUPLE,           // literal value

    ADD,            // +
    SUBTRACT,       // -
    MULTIPLY,       // *
    QUOTIENT,       // /

    ASSIGN,         // =
    COMMA,          // ,

    LBRACE,         // {
    RBRACE,         // }
    LPAREN,         // (
    RPAREN,         // )
    LBRACKET,         // [
    RBRACKET,         // ]

    FUNC,           // func
    MAIN,           // main (needed for func entrypoint)
    RETURN,         // return
    RETURN_VALUE,   // ->

} TokenType;

typedef struct {
    TokenType type;
    char* raw_text;        // raw source text
    int line;
    int column;
    
    // holds the actual token value and info in case it's needed (ex: IDENT)
    union {
        int int_value;
        float float_value;
        char* string_value;
        char char_value;
        bool bool_value;
    } value;
} Token;

void free_token(Token* token);

const char* token_type_to_string(TokenType type);

#endif