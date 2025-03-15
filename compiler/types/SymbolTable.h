#ifndef __SYMBOL_TABLE_H__
#define __SYMBOL_TABLE_H__

#include <string>
#include <unordered_map>
#include <vector>
#include <memory>
#include <set>
#include <optional>

// Forward declarations
class Type;
class ScopeNode;

// Reference/ownership types
enum class OwnershipType {
    Owner,      // Variable owns the memory (like Rust's ownership)
    Borrowed,   // Non-mutable reference/borrow
    MutBorrowed // Mutable reference/borrow
};

// Symbol represents a variable or function in the symbol table
struct Symbol {
    std::string name;
    std::shared_ptr<Type> type;
    
    // Source location info
    std::string filename;
    int definedAtLine;
    int definedAtCol;
    
    // Ownership information
    OwnershipType ownership;
    
    // If this is a reference, what variable does it point to?
    std::optional<std::string> pointsTo;
    
    // List of variables that reference this symbol
    std::set<std::string> referencedBy;
    
    // Places this variable is used
    struct Usage {
        std::string filename;
        int line;
        int col;
        bool isMutableUse; // Is this a use that mutates the variable?
    };
    std::vector<Usage> usages;
    
    Symbol(const std::string& name, std::shared_ptr<Type> type, 
           const std::string& file, int line, int col,
           OwnershipType ownership = OwnershipType::Owner)
        : name(name), type(type), filename(file), 
          definedAtLine(line), definedAtCol(col),
          ownership(ownership) {}
    
    bool isOwner() const { return ownership == OwnershipType::Owner; }
    bool isBorrowed() const { return ownership == OwnershipType::Borrowed; }
    bool isMutBorrowed() const { return ownership == OwnershipType::MutBorrowed; }
    bool isReference() const { return isBorrowed() || isMutBorrowed(); }
    
    void addUsage(const std::string& file, int line, int col, bool isMutable = false) {
        usages.push_back({file, line, col, isMutable});
    }
    
    void addReferencer(const std::string& varName) {
        referencedBy.insert(varName);
    }
    
    bool isReferencedAt(int line) const {
        for (const auto& usage : usages) {
            if (usage.line >= line) return true;
        }
        // Also check if other variables reference this one and are used at/after line
        return false; // This needs to be enhanced with a check of all references
    }
};

// A scope represents a block of code with its own variable namespace
class ScopeNode {
private:
    ScopeNode* parent;
    std::vector<std::unique_ptr<ScopeNode>> children;
    std::unordered_map<std::string, std::unique_ptr<Symbol>> symbols;
    int scopeId;
    
    // Scope start/end locations in source code
    std::string filename;
    int startLine;
    int startCol;
    int endLine;
    int endCol;
    
public:
    ScopeNode(ScopeNode* parent, int id, const std::string& file = "", 
             int startL = 0, int startC = 0, int endL = 0, int endC = 0)
        : parent(parent), scopeId(id), filename(file),
          startLine(startL), startCol(startC), endLine(endL), endCol(endC) {}
    
    int getId() const { return scopeId; }
    
    ScopeNode* getParent() const { return parent; }
    
    void addChild(std::unique_ptr<ScopeNode> child) {
        children.push_back(std::move(child));
    }
    
    Symbol* addSymbol(const std::string& name, std::shared_ptr<Type> type,
                    const std::string& file, int line, int col,
                    OwnershipType ownership = OwnershipType::Owner) {
        auto symbol = std::make_unique<Symbol>(name, type, file, line, col, ownership);
        Symbol* result = symbol.get();
        symbols[name] = std::move(symbol);
        return result;
    }
    
    Symbol* findSymbol(const std::string& name) {
        auto it = symbols.find(name);
        if (it != symbols.end()) {
            return it->second.get();
        }
        return nullptr;
    }
    
    Symbol* lookup(const std::string& name) {
        // Check in current scope
        Symbol* result = findSymbol(name);
        if (result) return result;
        
        // Check in parent scopes
        if (parent) return parent->lookup(name);
        
        return nullptr;
    }
    
    void setSourceLocation(const std::string& file, int startL, int startC, int endL, int endC) {
        filename = file;
        startLine = startL;
        startCol = startC;
        endLine = endL;
        endCol = endC;
    }
    
    bool contains(int line, int col) const {
        if (filename.empty()) return false;
        
        if (line > startLine && line < endLine) return true;
        if (line == startLine && col >= startCol) return true;
        if (line == endLine && col <= endCol) return true;
        
        return false;
    }
    
    const std::unordered_map<std::string, std::unique_ptr<Symbol>>& getSymbols() const {
        return symbols;
    }
    
    const std::vector<std::unique_ptr<ScopeNode>>& getChildren() const {
        return children;
    }
};

// The symbol table contains all scopes and manages the creation of new scopes
class SymbolTable {
private:
    std::unique_ptr<ScopeNode> rootScope;
    ScopeNode* currentScope;
    int nextScopeId;
    
public:
    SymbolTable() : nextScopeId(0) {
        rootScope = std::make_unique<ScopeNode>(nullptr, nextScopeId++);
        currentScope = rootScope.get();
    }
    
    // Create a new scope as a child of the current scope
    ScopeNode* enterScope(const std::string& file = "", int startL = 0, int startC = 0) {
        auto newScope = std::make_unique<ScopeNode>(currentScope, nextScopeId++, file, startL, startC);
        ScopeNode* result = newScope.get();
        currentScope->addChild(std::move(newScope));
        currentScope = result;
        return result;
    }
    
    // Exit the current scope and return to its parent
    ScopeNode* exitScope(int endL = 0, int endC = 0) {
        if (currentScope->getParent()) {
            // Update the end position
            if (endL > 0) {
                std::string file = currentScope->getParent()->getSymbols().empty() ? 
                                   "" : currentScope->getParent()->getSymbols().begin()->second->filename;
                currentScope->setSourceLocation(file, 
                                              currentScope->getSymbols().empty() ? 0 : 
                                              currentScope->getSymbols().begin()->second->definedAtLine,
                                              currentScope->getSymbols().empty() ? 0 : 
                                              currentScope->getSymbols().begin()->second->definedAtCol,
                                              endL, endC);
            }
            
            currentScope = currentScope->getParent();
        }
        return currentScope;
    }
    
    // Get the current scope
    ScopeNode* getCurrentScope() const {
        return currentScope;
    }
    
    // Get the root scope
    ScopeNode* getRootScope() const {
        return rootScope.get();
    }
    
    // Lookup a symbol by name
    Symbol* lookup(const std::string& name) {
        return currentScope->lookup(name);
    }
    
    // Add a new symbol to the current scope
    Symbol* addSymbol(const std::string& name, std::shared_ptr<Type> type,
                    const std::string& file, int line, int col,
                    OwnershipType ownership = OwnershipType::Owner) {
        return currentScope->addSymbol(name, type, file, line, col, ownership);
    }
    
    // Find the scope containing a specific position in the source
    ScopeNode* findScopeAt(const std::string& file, int line, int col) {
        // Recursive helper function
        std::function<ScopeNode*(ScopeNode*, const std::string&, int, int)> findInScope = 
            [&findInScope](ScopeNode* scope, const std::string& file, int line, int col) -> ScopeNode* {
                if (scope->contains(line, col)) {
                    // Check if any child contains the position
                    for (const auto& child : scope->getChildren()) {
                        ScopeNode* result = findInScope(child.get(), file, line, col);
                        if (result) return result;
                    }
                    return scope;
                }
                return nullptr;
            };
        
        return findInScope(rootScope.get(), file, line, col);
    }
};

#endif // __SYMBOL_TABLE_H__