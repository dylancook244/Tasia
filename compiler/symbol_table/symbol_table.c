#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "symbol_table.h"

#define INITIAL_CAPACITY 16
#define MAX_LOAD_FACTOR 0.75

// simple hash function for strings
static unsigned int hash_string(const char* str) {
    unsigned int hash = 5381;
    int c;

    while ((c = *str++)) {
        hash = ((hash << 5) + hash) + c; // hash * 33 + c
    }

    return hash;
}

// Free a symbol and its contents
static void free_symbol(Symbol* symbol) {
    if (!symbol) return;
    
    if (symbol->name) free(symbol->name);
    if (symbol->type) free(symbol->type);
    
    if (symbol->param_types) {
        for (int i = 0; i < symbol->param_count; i++) {
            if (symbol->param_types[i]) free(symbol->param_types[i]);
        }
        free(symbol->param_types);
    }
    
    free(symbol);
}

// create a new scope
static Scope* create_scope(int level, Scope* parent) {
    Scope* scope = malloc(sizeof(Scope));
    if (!scope) return NULL;

    scope->symbols = calloc(INITIAL_CAPACITY, sizeof(Symbol*));
    if (!scope->symbols) {
        free(scope);
        return NULL;
    }

    scope->capacity = INITIAL_CAPACITY;
    scope->symbol_count = 0;
    scope->level = level;
    scope->parent = parent;
    scope->children = NULL;
    scope->child_count = 0;
    scope->child_capacity = 0;

    return scope;
}

static char* my_strdup(const char* str) {
    size_t len = strlen(str) + 1;
    char* new_str = malloc(len);
    if (new_str) {
        memcpy(new_str, str, len);
    }
    return new_str;
}

// add child scope to parent scope
static void add_child_scope(Scope* parent, Scope* child) {
    if (parent->child_count >= parent->child_capacity) {
        int new_capacity = parent->child_capacity == 0 ? 4 : parent->child_capacity * 2;
        Scope** new_children = realloc(parent->children, sizeof(Scope*) * new_capacity);
        if (!new_children) return;

        parent->children = new_children;
        parent->child_capacity = new_capacity;
    }

    parent->children[parent->child_count++] = child;
}


// Create a new symbol table
SymbolTable* create_symbol_table() {
    SymbolTable* table = malloc(sizeof(SymbolTable));
    if (!table) return NULL;
    
    table->global_scope = create_scope(0, NULL);
    if (!table->global_scope) {
        free(table);
        return NULL;
    }
    
    table->current_scope = table->global_scope;
    return table;
}

// Enter a new scope
void enter_scope(SymbolTable* table) {
    if (!table) return;
    
    Scope* new_scope = create_scope(table->current_scope->level + 1, table->current_scope);
    if (!new_scope) return;
    
    add_child_scope(table->current_scope, new_scope);
    table->current_scope = new_scope;
}

// Exit the current scope
void exit_scope(SymbolTable* table) {
    if (!table || table->current_scope == table->global_scope) return;
    
    table->current_scope = table->current_scope->parent;
}

// Create a new symbol
static Symbol* create_symbol(const char* name, SymbolKind kind, const char* type) {
    Symbol* symbol = malloc(sizeof(Symbol));
    if (!symbol) return NULL;
    
    symbol->name = strdup(name);
    if (!symbol->name) {
        free(symbol);
        return NULL;
    }
    
    symbol->kind = kind;
    symbol->type = type ? strdup(type) : NULL;
    symbol->is_mutable = false;
    symbol->is_initialized = false;
    symbol->scope_level = 0;
    symbol->param_types = NULL;
    symbol->param_count = 0;
    symbol->is_borrowed = false;
    symbol->borrowed_from = NULL;
    symbol->next = NULL;
    
    return symbol;
}

Symbol* add_symbol(SymbolTable* table, const char* name, SymbolKind kind, const char* type, char** param_types, int param_count) {
    // not sure why name would be null but I'll leave that there anyway
    if (!table || !name) return NULL;

    // check if symobl already exists in current scope
    if (lookup_symbol_current_scope(table, name)) {
        printf("Error: Symbol '%s' already defined in current scope\n", name);
        return NULL;
    }

    // create new symbol
    Symbol* symbol = create_symbol(name, kind, type);
    if (!symbol) return NULL;

    symbol->scope_level = table->current_scope->level;

    // add func parameters if this is a function
    if (kind == SYMBOL_FUNCTION && param_count > 0 && param_types) {
        symbol->param_types = malloc(sizeof(char*) * param_count);
        if (!symbol->param_types) {
            free_symbol(symbol);
            return NULL;
        }

        for (int i = 0; i < param_count; i++) {
            symbol->param_types[i] = my_strdup(param_types[i]);
        }
        symbol->param_count = param_count;
    }

    // add to hash table
    unsigned int hash = hash_string(name) % table->current_scope->capacity;
    symbol->next = table->current_scope->symbols[hash];
    table->current_scope->symbols[hash] = symbol;
    table->current_scope->symbol_count++;

    return symbol;
}

// Look up a symbol in the current scope only
Symbol* lookup_symbol_current_scope(SymbolTable* table, const char* name) {
    if (!table || !name) return NULL;
    
    unsigned int hash = hash_string(name) % table->current_scope->capacity;
    Symbol* current = table->current_scope->symbols[hash];
    
    while (current) {
        if (strcmp(current->name, name) == 0) {
            return current;
        }
        current = current->next;
    }
    
    return NULL;
}


// Look up a symbol in current and parent scopes
Symbol* lookup_symbol(SymbolTable* table, const char* name) {
    if (!table || !name) return NULL;
    
    Scope* current_scope = table->current_scope;
    
    while (current_scope) {
        unsigned int hash = hash_string(name) % current_scope->capacity;
        Symbol* current = current_scope->symbols[hash];
        
        while (current) {
            if (strcmp(current->name, name) == 0) {
                return current;
            }
            current = current->next;
        }
        
        current_scope = current_scope->parent;
    }
    
    return NULL;
}

// Free a scope and all its contents
static void free_scope(Scope* scope) {
    if (!scope) return;
    
    // Free all symbols in this scope
    for (int i = 0; i < scope->capacity; i++) {
        Symbol* current = scope->symbols[i];
        while (current) {
            Symbol* next = current->next;
            free_symbol(current);
            current = next;
        }
    }
    
    // Free the symbols array
    free(scope->symbols);
    
    // Recursively free all child scopes
    for (int i = 0; i < scope->child_count; i++) {
        free_scope(scope->children[i]);
    }
    
    // Free the children array
    if (scope->children) free(scope->children);
    
    // Free the scope itself
    free(scope);
}

// Free the symbol table and all its contents
void free_symbol_table(SymbolTable* table) {
    if (!table) return;
    
    free_scope(table->global_scope);
    free(table);
}

// Ownership system functions
bool check_ownership(SymbolTable* table, const char* name, bool is_mutable_access) {
    Symbol* symbol = lookup_symbol(table, name);
    if (!symbol) return false;
    
    // If this is a mutable access, check if it's allowed
    if (is_mutable_access) {
        if (!symbol->is_mutable) {
            printf("Error: Cannot mutably access immutable variable '%s'\n", name);
            return false;
        }
        
        if (symbol->is_borrowed) {
            printf("Error: Cannot mutably access variable '%s' while it's borrowed\n", name);
            return false;
        }
    }
    
    // For immutable access, check if there are mutable borrows
    else {
        if (symbol->is_borrowed && symbol->is_mutable) {
            printf("Error: Cannot access variable '%s' while it's mutably borrowed\n", name);
            return false;
        }
    }
    
    return true;
}

bool borrow_symbol(SymbolTable* table, const char* name, bool is_mutable, Symbol* borrower) {
    Symbol* symbol = lookup_symbol(table, name);
    if (!symbol) return false;
    
    // Check if borrowing is allowed
    if (is_mutable) {
        if (!symbol->is_mutable) {
            printf("Error: Cannot mutably borrow immutable variable '%s'\n", name);
            return false;
        }
        
        if (symbol->is_borrowed) {
            printf("Error: Cannot mutably borrow variable '%s' while it's already borrowed\n", name);
            return false;
        }
    } else {
        if (symbol->is_borrowed && symbol->is_mutable) {
            printf("Error: Cannot borrow variable '%s' while it's mutably borrowed\n", name);
            return false;
        }
    }
    
    // Set up borrowing relationship
    symbol->is_borrowed = true;
    borrower->borrowed_from = symbol;
    
    return true;
}

void release_borrow(SymbolTable* table, Symbol* borrower) {
    if (!borrower || !borrower->borrowed_from) return;
    
    Symbol* lender = borrower->borrowed_from;
    
    // If this was the only borrow, mark as not borrowed
    // In a real implementation, you'd track multiple borrows
    lender->is_borrowed = false;
    
    borrower->borrowed_from = NULL;
}

void dump_scope(Scope* scope, int indent) {
    printf("%*sScope Level %d:\n", indent, "", scope->level);
    
    // Print all symbols in this scope
    for (int i = 0; i < scope->capacity; i++) {
        Symbol* symbol = scope->symbols[i];
        while (symbol) {
            printf("%*s- %s: ", indent + 2, "", symbol->name);
            
            switch (symbol->kind) {
                case SYMBOL_VARIABLE:
                    printf("Variable, Type: %s, Mutable: %s, Initialized: %s\n",
                           symbol->type ? symbol->type : "none",
                           symbol->is_mutable ? "yes" : "no",
                           symbol->is_initialized ? "yes" : "no");
                    break;
                    
                case SYMBOL_FUNCTION:
                    printf("Function, Return Type: %s, Params: %d\n",
                           symbol->type ? symbol->type : "void",
                           symbol->param_count);
                    break;
                    
                default:
                    printf("Unknown kind\n");
                    break;
            }
            
            symbol = symbol->next;
        }
    }
    
    // Recursively print child scopes
    for (int i = 0; i < scope->child_count; i++) {
        dump_scope(scope->children[i], indent + 4);
    }
}

void dump_symbol_table(SymbolTable* table) {
    printf("\n=== SYMBOL TABLE DUMP ===\n");
    
    // Implement a recursive function to print all symbols in all scopes
    dump_scope(table->global_scope, 0);
    
    printf("=== END SYMBOL TABLE DUMP ===\n\n");
}

