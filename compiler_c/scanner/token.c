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
        case END_OF_FILE:  return "EOF";
        case END_OF_LINE:  return "EOL";
        case COMMENT:      return "COMMENT";

        case IDENT:        return "IDENT";

        case ADD:          return "ADD";
        case SUBTRACT:     return "SUBTRACT";
        case MULTIPLY:     return "MULTIPLY";
        case QUOTIENT:     return "QUOTIENT";

        case ASSIGN:       return "ASSIGN";

        case INT:          return "INT";  
        case FLOAT:        return "FLOAT"; 
        case STRING:       return "STRING";
        case CHAR:         return "CHAR";
        case BOOL:         return "BOOL";

        case LBRACE:       return "LBRACE";
        case RBRACE:       return "RBRACE";
        case LPAREN:       return "LPAREN";
        case RPAREN:       return "RPAREN";
        case LBRACKET:     return "LBRACKET";
        case RBRACKET:     return "RBRACKET";

        case FUNC:         return "FUNC";
        case RETURN:       return "RETURN";
        
        default:           return "UNKNOWN";
    }
}