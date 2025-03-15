#ifndef __TYPES_H__
#define __TYPES_H__

#include <string>
#include <memory>
#include <vector>
#include <unordered_map>

// Forward declaration
class Type;

// Type kinds
enum class TypeKind {
    Void,
    Integer,
    Float,
    Boolean,
    String,
    Array,
    Slice,     // Reference to part of an array
    Reference, // Reference to another type
    Function,
    Struct,
    Enum,
    Unknown
};

// Class for lifetime information
class Lifetime {
public:
    enum class Kind {
        Static,    // 'static lifetime
        Function,  // Tied to a function scope
        Block,     // Tied to a specific block
        Parameter, // Tied to a parameter
        Inferred   // To be inferred by the compiler
    };
    
private:
    Kind kind;
    int scopeId;  // Which scope this lifetime is tied to (if applicable)
    std::string name; // For named lifetimes (mostly internal use)
    
public:
    Lifetime(Kind kind = Kind::Inferred, int scopeId = -1, const std::string& name = "")
        : kind(kind), scopeId(scopeId), name(name) {}
    
    Kind getKind() const { return kind; }
    int getScopeId() const { return scopeId; }
    const std::string& getName() const { return name; }
    
    bool outlives(const Lifetime& other) const {
        if (kind == Kind::Static) return true;
        if (other.kind == Kind::Static) return false;
        
        // Same scopes have equal lifetimes
        if (scopeId == other.scopeId && kind == other.kind) return true;
        
        // For different scopes, we need more context (parent-child relationship)
        // To be extended in the borrow checker implementation
        return false;
    }
    
    std::string toString() const {
        switch (kind) {
            case Kind::Static: return "'static";
            case Kind::Inferred: return "'_";
            default: return "'" + name;
        }
    }
    
    static Lifetime createStatic() {
        return Lifetime(Kind::Static, -1, "static");
    }
    
    static Lifetime createForScope(int scopeId) {
        return Lifetime(Kind::Block, scopeId, "scope_" + std::to_string(scopeId));
    }
    
    static Lifetime createInferred() {
        return Lifetime(Kind::Inferred, -1, "");
    }
};

// Base class for all types
class Type {
protected:
    TypeKind kind;
    std::string name;
    bool mutable_;
    
public:
    Type(TypeKind kind, const std::string& name, bool mutable_ = true)
        : kind(kind), name(name), mutable_(mutable_) {}
    
    virtual ~Type() = default;
    
    TypeKind getKind() const { return kind; }
    const std::string& getName() const { return name; }
    bool isMutable() const { return mutable_; }
    
    void setMutable(bool m) { mutable_ = m; }
    
    virtual std::string toString() const {
        std::string result = name;
        if (!mutable_) result = "const " + result;
        return result;
    }
    
    // Type compatibility checking
    virtual bool isCompatibleWith(const Type* other) const {
        return kind == other->kind && name == other->name;
    }
    
    // Factory methods for common types
    static std::shared_ptr<Type> createVoid() {
        return std::make_shared<Type>(TypeKind::Void, "void", false);
    }
    
    static std::shared_ptr<Type> createInt() {
        return std::make_shared<Type>(TypeKind::Integer, "int");
    }
    
    static std::shared_ptr<Type> createFloat() {
        return std::make_shared<Type>(TypeKind::Float, "float");
    }
    
    static std::shared_ptr<Type> createBool() {
        return std::make_shared<Type>(TypeKind::Boolean, "bool");
    }
    
    static std::shared_ptr<Type> createString() {
        return std::make_shared<Type>(TypeKind::String, "string");
    }
    
    static std::shared_ptr<Type> createUnknown() {
        return std::make_shared<Type>(TypeKind::Unknown, "<unknown>");
    }
};

// Reference type - points to another type
class ReferenceType : public Type {
private:
    std::shared_ptr<Type> pointeeType;
    Lifetime lifetime;
    
public:
    ReferenceType(std::shared_ptr<Type> pointee, bool mutable_ = false, 
                 Lifetime lifetime = Lifetime::createInferred())
        : Type(TypeKind::Reference, 
            std::string("&") + (mutable_ ? "mut " : "") + pointee->getName()
        ),
          pointeeType(pointee), lifetime(lifetime) {}
    
    std::shared_ptr<Type> getPointeeType() const { return pointeeType; }
    const Lifetime& getLifetime() const { return lifetime; }
    
    void setLifetime(const Lifetime& lt) { lifetime = lt; }
    
    virtual std::string toString() const override {
        std::string result = "&";
        if (lifetime.getKind() != Lifetime::Kind::Inferred) {
            result += lifetime.toString() + " ";
        }
        if (isMutable()) result += "mut ";
        result += pointeeType->toString();
        return result;
    }
    
    virtual bool isCompatibleWith(const Type* other) const override {
        if (other->getKind() != TypeKind::Reference) return false;
        
        const ReferenceType* otherRef = static_cast<const ReferenceType*>(other);
        
        // Immutable refs can be coerced from mutable refs, but not vice versa
        if (isMutable() && !otherRef->isMutable()) return false;
        
        // Check the underlying type compatibility
        return pointeeType->isCompatibleWith(otherRef->getPointeeType().get());
    }
};

// Array type
class ArrayType : public Type {
private:
    std::shared_ptr<Type> elementType;
    size_t size; // Fixed size or 0 for dynamic
    
public:
    ArrayType(std::shared_ptr<Type> elemType, size_t size = 0)
        : Type(TypeKind::Array, 
              elemType->getName() + "[" + (size > 0 ? std::to_string(size) : "") + "]"),
          elementType(elemType), size(size) {}
    
    std::shared_ptr<Type> getElementType() const { return elementType; }
    size_t getSize() const { return size; }
    bool isFixedSize() const { return size > 0; }
    
    virtual std::string toString() const override {
        return elementType->toString() + "[" + (size > 0 ? std::to_string(size) : "") + "]";
    }
    
    virtual bool isCompatibleWith(const Type* other) const override {
        if (other->getKind() != TypeKind::Array) return false;
        
        const ArrayType* otherArray = static_cast<const ArrayType*>(other);
        
        // Check size compatibility
        if (isFixedSize() && otherArray->isFixedSize() && size != otherArray->size) {
            return false;
        }
        
        // Check element type compatibility
        return elementType->isCompatibleWith(otherArray->getElementType().get());
    }
};

// Slice type - reference to part of an array
class SliceType : public Type {
private:
    std::shared_ptr<Type> elementType;
    Lifetime lifetime;
    
public:
    SliceType(std::shared_ptr<Type> elemType, bool mutable_ = false,
             Lifetime lifetime = Lifetime::createInferred())
        : Type(TypeKind::Slice, 
            std::string("&") + (mutable_ ? "mut " : "") + "[]" + elemType->getName()),
          elementType(elemType), lifetime(lifetime) {}
    
    std::shared_ptr<Type> getElementType() const { return elementType; }
    const Lifetime& getLifetime() const { return lifetime; }
    
    void setLifetime(const Lifetime& lt) { lifetime = lt; }
    
    virtual std::string toString() const override {
        std::string result = "&";
        if (lifetime.getKind() != Lifetime::Kind::Inferred) {
            result += lifetime.toString() + " ";
        }
        if (isMutable()) result += "mut ";
        result += "[" + elementType->toString() + "]";
        return result;
    }
    
    virtual bool isCompatibleWith(const Type* other) const override {
        if (other->getKind() != TypeKind::Slice) return false;
        
        const SliceType* otherSlice = static_cast<const SliceType*>(other);
        
        // Immutable refs can be coerced from mutable refs, but not vice versa
        if (isMutable() && !otherSlice->isMutable()) return false;
        
        // Check the underlying element type compatibility
        return elementType->isCompatibleWith(otherSlice->getElementType().get());
    }
};

// Function type
class FunctionType : public Type {
private:
    std::shared_ptr<Type> returnType;
    std::vector<std::shared_ptr<Type>> paramTypes;
    
public:
    FunctionType(std::shared_ptr<Type> retType,
                std::vector<std::shared_ptr<Type>> params)
        : Type(TypeKind::Function, "fn"), returnType(retType), paramTypes(std::move(params)) {
        // Construct full name
        name = "fn(";
        for (size_t i = 0; i < paramTypes.size(); ++i) {
            if (i > 0) name += ", ";
            name += paramTypes[i]->toString();
        }
        name += ") -> ";
        name += returnType->toString();
    }
    
    std::shared_ptr<Type> getReturnType() const { return returnType; }
    
    const std::vector<std::shared_ptr<Type>>& getParamTypes() const {
        return paramTypes;
    }
    
    virtual std::string toString() const override {
        return name;
    }
    
    virtual bool isCompatibleWith(const Type* other) const override {
        if (other->getKind() != TypeKind::Function) return false;
        
        const FunctionType* otherFunc = static_cast<const FunctionType*>(other);
        
        // Check return type compatibility
        if (!returnType->isCompatibleWith(otherFunc->getReturnType().get())) {
            return false;
        }
        
        // Check parameter count
        if (paramTypes.size() != otherFunc->paramTypes.size()) {
            return false;
        }
        
        // Check each parameter type
        for (size_t i = 0; i < paramTypes.size(); ++i) {
            if (!paramTypes[i]->isCompatibleWith(otherFunc->paramTypes[i].get())) {
                return false;
            }
        }
        
        return true;
    }
};

// Type registry to reuse types
class TypeRegistry {
private:
    // Cache of common types
    std::shared_ptr<Type> voidType;
    std::shared_ptr<Type> intType;
    std::shared_ptr<Type> floatType;
    std::shared_ptr<Type> boolType;
    std::shared_ptr<Type> stringType;
    
    // Caches for complex types
    std::unordered_map<std::string, std::shared_ptr<Type>> typeCache;
    
public:
    TypeRegistry() {
        voidType = Type::createVoid();
        intType = Type::createInt();
        floatType = Type::createFloat();
        boolType = Type::createBool();
        stringType = Type::createString();
        
        // Add built-in types to cache
        typeCache["void"] = voidType;
        typeCache["int"] = intType;
        typeCache["float"] = floatType;
        typeCache["bool"] = boolType;
        typeCache["string"] = stringType;
    }
    
    // Get basic types
    std::shared_ptr<Type> getVoid() const { return voidType; }
    std::shared_ptr<Type> getInt() const { return intType; }
    std::shared_ptr<Type> getFloat() const { return floatType; }
    std::shared_ptr<Type> getBool() const { return boolType; }
    std::shared_ptr<Type> getString() const { return stringType; }
    
    // Create or retrieve array type
    std::shared_ptr<Type> getArrayType(std::shared_ptr<Type> elemType, size_t size = 0) {
        std::string key = "array_" + elemType->getName() + "_" + std::to_string(size);
        auto it = typeCache.find(key);
        if (it != typeCache.end()) {
            return it->second;
        }
        
        auto arrayType = std::make_shared<ArrayType>(elemType, size);
        typeCache[key] = arrayType;
        return arrayType;
    }
    
    // Create or retrieve slice type
    std::shared_ptr<Type> getSliceType(std::shared_ptr<Type> elemType, bool mutable_ = false) {
        std::string key = std::string("slice_") + (mutable_ ? "mut_" : "") + elemType->getName();
        auto it = typeCache.find(key);
        if (it != typeCache.end()) {
            return it->second;
        }
        
        auto sliceType = std::make_shared<SliceType>(elemType, mutable_);
        typeCache[key] = sliceType;
        return sliceType;
    }
    
    // Create or retrieve reference type
    std::shared_ptr<Type> getReferenceType(std::shared_ptr<Type> pointeeType, bool mutable_ = false) {
        std::string key = std::string("ref_") + (mutable_ ? "mut_" : "") + pointeeType->getName();
        auto it = typeCache.find(key);
        if (it != typeCache.end()) {
            return it->second;
        }
        
        auto refType = std::make_shared<ReferenceType>(pointeeType, mutable_);
        typeCache[key] = refType;
        return refType;
    }
    
    // Create function type (not cached for simplicity)
    std::shared_ptr<Type> createFunctionType(
        std::shared_ptr<Type> returnType,
        std::vector<std::shared_ptr<Type>> paramTypes
    ) {
        return std::make_shared<FunctionType>(returnType, std::move(paramTypes));
    }
    
    // Register a custom type
    void registerType(const std::string& name, std::shared_ptr<Type> type) {
        typeCache[name] = type;
    }
    
    // Lookup a type by name
    std::shared_ptr<Type> lookupType(const std::string& name) {
        auto it = typeCache.find(name);
        if (it != typeCache.end()) {
            return it->second;
        }
        return nullptr;
    }
};

#endif // __TYPES_H__