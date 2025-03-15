#ifndef TOKEN_H
#define TOKEN_H

// token name in program along with user syntax

enum Token {

    ILLEGAL = 0,    // N/A
    END_OF_FILE,    // EOF
    END_OF_LINE,    // \n
    COMMENT,        // #

    IDENT,          // user specified

    ADD,            // +
    SUBTRACT,       // - 
    MULTIPLY,       // *
    QUOTIENT,       // /

    ASSIGN,         // =

    INT,            // int
    FLOAT,          // float
    STRING,         // string
    CHAR,           // char
    BOOL,           // bool

    LBRACE,         // {
    RBRACE,         // }
    LPAREN,         // (
    RPAREN,         // )
    LBRACE,         // [
    RBRACE,         // ]

    FUNC,           // func
    RETURN,         // return

}

#endif