#ifndef __AST_NODE_H__
#define __AST_NODE_H__

#include <string>

// Basic source location tracking
struct SourceLocation {
    std::string filename;
    int line;
    int column;
    
    SourceLocation() : line(0), column(0) {}
    SourceLocation(const std::string &file, int l, int c) 
        : filename(file), line(l), column(c) {}
};

// Base class for all AST nodes
class ASTNode {
protected:
    SourceLocation location;
    
public:
    ASTNode() = default;
    virtual ~ASTNode() = default;
    
    // Source location tracking
    void setLocation(const SourceLocation& loc) { location = loc; }
    const SourceLocation &getLocation() const { return location; }
    
    // For debugging
    virtual std::string toString() const = 0;
};

#endif // __AST_NODE_H__