#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>

#include "../parser/parser.h"
#include "../scanner/scanner.h"
#include "../ast/ast.h"
#include "../symbol_table/symbol_table.h"
#include "../codegen/codegen.h"

void build_executable(const char* llvm_file, const char* output_file) {
    printf("\nBuilding executable from LLVM bitcode...\n");
    
    // On macOS with Homebrew, use the full path to llc
    char llc_command[2048];
    #ifdef __APPLE__
    sprintf(llc_command, "/opt/homebrew/Cellar/llvm/19.1.7_1/bin/llc -filetype=obj %s -o %s.o", 
            llvm_file, output_file);
    #else
    sprintf(llc_command, "llc -filetype=obj %s -o %s.o", llvm_file, output_file);
    #endif
    
    printf("Running: %s\n", llc_command);
    system(llc_command);
    
    // Link object file to create executable
    char clang_command[2048];
    #ifdef __APPLE__
    sprintf(clang_command, "/opt/homebrew/Cellar/llvm/19.1.7_1/bin/clang %s.o -o %s", 
            output_file, output_file);
    #else
    sprintf(clang_command, "clang %s.o -o %s", output_file, output_file);
    #endif
    
    printf("Running: %s\n", clang_command);
    system(clang_command);
    
    printf("Executable created: %s\n", output_file);
}

const char* RUNNER_C_CONTENT = 
"#include <stdio.h>\n\n"
"// The entry point for Tasia programs\n"
"extern int TASIA_ENTRY_FUNCTION(void);\n\n"
"int main(int argc, char** argv) {\n"
"    int result = TASIA_ENTRY_FUNCTION();\n"
"    printf(\"Program returned: %d\\n\", result);\n"
"    return 0;\n"
"}\n";

bool write_runner_to_file(const char* path) {
    FILE* file = fopen(path, "w");
    if (!file) {
        printf("Error: couldn't create temporary runner file\n");
        return false;
    }
    
    fprintf(file, "%s", RUNNER_C_CONTENT);
    fclose(file);
    return true;
}

void compile(char* filepath) {
    printf("\nCompiling %s", filepath);

    // Initialize scanner
    Scanner* scanner = init_scanner(filepath);

    // Parse program and build AST + symbol table
    AstNode* ast = parse_program(scanner);
    SymbolTable* symbol_table = ((ProgramNode*)ast)->symbol_table;

    // Display AST for debugging
    printf("\n\n=== AST DUMP ===\n");
    dump_ast(ast, 0);
    
    // Semantic checking
    if (!check_semantics(ast, symbol_table)) {
        printf("Semantic errors found. Aborting compilation.\n");
        free_ast_node(ast);
        free_symbol_table(symbol_table);
        free_scanner(scanner);
        return;
    }
    
    // Generate output filenames
    char output_file[1024];
    strcpy(output_file, filepath);
    char* dot = strrchr(output_file, '.');
    if (dot) *dot = '\0';
    
    char obj_file[1024];
    sprintf(obj_file, "%s.o", output_file);
    
    char ll_file[1024];
    sprintf(ll_file, "%s.ll", output_file);
    
    // Generate code
    generate_code(ast, symbol_table, ll_file);

    // Generate object file
    char llc_command[2048];
    #ifdef __APPLE__
    sprintf(llc_command, "/opt/homebrew/Cellar/llvm/19.1.7_1/bin/llc -filetype=obj %s -o %s", ll_file, obj_file);
    #else
    sprintf(llc_command, "llc -filetype=obj %s -o %s", ll_file, obj_file);
    #endif
    
    printf("Running: %s\n", llc_command);
    system(llc_command);
    
    // Create temp runner.c file
    char temp_runner[1024];
    sprintf(temp_runner, "temp_runner_%d.c", (int)time(NULL));
    
    if (!write_runner_to_file(temp_runner)) {
        printf("Failed to create runner file\n");
        return;
    }
    
    // Link object file with runner to create executable
    char clang_command[2048];
    sprintf(clang_command, "clang %s %s -o %s", temp_runner, obj_file, output_file);
    
    printf("Running: %s\n", clang_command);
    int link_result = system(clang_command);
    
    // Clean up temp files
    remove(temp_runner);
    
    if (link_result == 0) {
        printf("Executable created: %s\n", output_file);
    } else {
        printf("Error: linking failed\n");
    }
    
    // Clean up
    free_scanner(scanner);
    free_ast_node(ast);
    free_symbol_table(symbol_table);
}