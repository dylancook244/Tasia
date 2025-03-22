#include <stdlib.h>
#include "token.h"

void free_token(Token* token) {
    if (token) {
        if (token->raw_text) {
            free(token->raw_text);
        }
        // Free string_value if needed
        if (token->type == STRING && token->value.string_value) {
            free(token->value.string_value);
        }
        free(token);
    }
}

// Convert token type to string for debugging and error messages
const char* token_type_to_string(TokenType type) {
    switch (type) {
        case ILLEGAL:      return "ILLEGAL";
        case END_OF_FILE:  return "END_OF_FILE";
        case END_OF_LINE:  return "END_OF_LINE";
        case COMMENT:      return "COMMENT";

        case IDENT:        return "IDENT";

        case INT:          return "INT";  
        case FLOAT:        return "FLOAT"; 
        case CHAR:         return "CHAR";
        case STRING:       return "STRING";
        case BOOL:         return "BOOL";

        case INT_LITERAL:          return "INT_LITERAL";  
        case FLOAT_LITERAL:        return "FLOAT_LITERAL"; 
        case CHAR_LITERAL:         return "CHAR_LITERAL";
        case STRING_LITERAL:       return "STRING_LITERAL";
        case BOOL_LITERAL:         return "BOOL_LITERAL";

        case ARRAY:         return "ARRAY";
        case LIST:         return "LIST";
        case TUPLE:         return "TUPLE";

        case ADD:          return "ADD";
        case SUBTRACT:     return "SUBTRACT";
        case MULTIPLY:     return "MULTIPLY";
        case QUOTIENT:     return "QUOTIENT";

        case ASSIGN:       return "ASSIGN";
        case COMMA:        return "COMMA";

        case LBRACE:       return "LBRACE";
        case RBRACE:       return "RBRACE";
        case LPAREN:       return "LPAREN";
        case RPAREN:       return "RPAREN";
        case LBRACKET:     return "LBRACKET";
        case RBRACKET:     return "RBRACKET";

        case FUNC:         return "FUNC";
        case MAIN:         return "MAIN";
        case RETURN:       return "RETURN";
        case RETURN_VALUE:       return "RETURN_VALUE";

        default:           return "UNKNOWN";
    }
}