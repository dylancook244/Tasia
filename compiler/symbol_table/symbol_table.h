#ifndef SYMBOL_TABLE_H
#define SYMBOL_TABLE_H

#include <stdbool.h>
#include "../ast/ast.h"

typedef enum {
    SYMBOL_VARIABLE,
    SYMBOL_FUNCTION,
    SYMBOL_TYPE,
    SYMBOL_MODULE
} SymbolKind;

typedef struct Symbol Symbol;
typedef struct Scope Scope;
typedef struct SymbolTable SymbolTable;

struct Symbol {
    char* name;            // name of symbol
    SymbolKind kind;       // kind of symbol
    char* type; 
    bool is_initialized;
    int scope_level;

    bool is_mutable;
    bool is_borrowed;
    Symbol* borrowed_from; // if borrowed

    // for functions
    char** param_types;
    int param_count;

    Symbol* next;           // pointer to next symbol (for hash collections)
};

struct Scope {
    Symbol** symbols;   // hash table of symbols in this scope
    int symbol_count;
    int capacity;
    int level;
    Scope* parent;
    Scope** children;
    int child_count;
    int child_capacity;
};

struct SymbolTable {
    Scope* global_scope;
    Scope* current_scope;
};

SymbolTable* create_symbol_table();

void enter_scope(SymbolTable* table);
void exit_scope(SymbolTable* table);

Symbol* add_symbol(SymbolTable* table, const char* name, SymbolKind kind, const char* type, char** param_types, int param_count);
Symbol* lookup_symbol(SymbolTable* table, const char* name);
Symbol* lookup_symbol_current_scope(SymbolTable* table, const char* name);

void free_symbol_table(SymbolTable* table);

// basic borrow checking
bool check_ownership(SymbolTable* table, const char* name, bool is_mutable_access);
bool borrow_symbol(SymbolTable* table, const char* name, bool is_mutable, Symbol* borrower);
void release_borrow(SymbolTable* table, Symbol* borrower);

void dump_symbol_table(SymbolTable* table);

#endif
