#ifndef __EXPR_AST_H__
#define __EXPR_AST_H__

#include "ASTNode.h"

#include <optional>
#include <string>

class ExprAST : public ASTNode {
private:
    bool isOwner = false;
    bool isMutable = true;
    std::optional<std::string> pointsTo;
    
public:
    ExprAST() = default;
    virtual ~ExprAST() = default;
    
    bool getIsOwner() const { return isOwner; }
    void setIsOwner(bool owner) { isOwner = owner; }
    
    bool getIsMutable() const { return isMutable; }
    void setIsMutable(bool mutable_) { isMutable = mutable_; }
    
    bool isReference() const { return !isOwner; }
    
    std::optional<std::string> getPointsTo() const { return pointsTo; }
    void setPointsTo(const std::string& target) { pointsTo = target; }
    void clearPointsTo() { pointsTo.reset(); }
};

#endif // __EXPR_AST_H__