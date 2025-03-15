#include <stdio.h>
#include "parser.h"
#include "../scanner/scanner.h"

void parseProgram(Scanner* scanner) {
    printf("\n\n=== CHAR OUTPUT ===\n");

    while (scanner->current_char != '\0') {
        printf("\n%c", scanner->current_char);
        advance_char(scanner);
    }
}