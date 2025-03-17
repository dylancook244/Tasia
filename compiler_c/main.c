#include <stdio.h>
// #include <unistd.h>

#ifdef _WIN32
    #include <direct.h>
    #define getcwd _getcwd // stupid MSFT "deprecation" warning
#else
    #include <unistd.h>
#endif

#include "parser/parser.h"
#include "scanner/scanner.h"
#include "ast/ast.h"

void compile(char* filepath) {
    printf("\nI'm getting the path %s", filepath);

    // make scanner
    Scanner* scanner = init_scanner(filepath);

    AstNode* ast = parse_program(scanner);

    printf("\n\n=== AST DUMP ===\n");
    dump_ast(ast, 0);
    
    // clean up
    free_scanner(scanner);

    // generate code from ast into a file
    // generate_code(ast, output_file);

    // Free ast
    free_ast_node(ast);

    // Free Symbol Table
    // free_symbol_table()
}

char* get_full_path(char* filename) {
    // allocate space on stack for current working directory
    static char cwd[1024];

    // allocate space on stack for full filepath
    static char filepath[1024];

    // gets cwd and returns an error if we can't access it
    if (getcwd(cwd, sizeof(cwd)) == NULL) {
        printf("getcwd error, couldn't access current working directory\n");
        return NULL;
    }

    // apparently sprintf doesn't actually print
    // it's what allocates stack memory. weird
    sprintf(filepath, "%s/%s", cwd, filename);

    return filepath;
}

int main(int argc, char** argv) {

    if (argc < 2) {
        printf("please enter an argument for tasia\n");
        printf("Usage: tasia <command> <file>");
        return 1;
    }

    char* my_command = argv[1];

    char buffer [100];
    
    if (argc >= 3) {
        char* filename = argv[2];

        printf("you entered: tasia %s %s", my_command, filename);

        char* filepath = get_full_path(filename);
        if (filepath != NULL) {
            compile(filepath);
        } else {
            printf("could not find the file %s", filepath);
            return 1;
        }

        printf("%s\n", buffer);

        return 0;
    }

    printf("you entered: tasia %s", my_command);
    printf("%s\n", buffer);


    return 0;

}