#include "CodeGenerator.h"
#include <sstream>

void CodeGenerator::reportError(const std::string& msg, const SourceLocation& loc) {
    std::stringstream ss;
    ss << loc.filename << ":" << loc.line << ":" << loc.column << ": error: " << msg;
    errors.push_back(ss.str());
}

llvm::Value* CodeGenerator::generateCode(ExprAST* expr) {
    if (!expr) return nullptr;
    
    // Use dynamic_cast to determine the concrete type of the expression
    if (auto* numExpr = dynamic_cast<NumberExprAST*>(expr)) {
        return generateCode(numExpr);
    } else if (auto* varExpr = dynamic_cast<VariableExprAST*>(expr)) {
        return generateCode(varExpr);
    } else if (auto* binExpr = dynamic_cast<BinaryExprAST*>(expr)) {
        return generateCode(binExpr);
    } else if (auto* callExpr = dynamic_cast<CallExprAST*>(expr)) {
        return generateCode(callExpr);
    }
    
    reportError("Unknown expression type", expr->getLocation());
    return nullptr;
}

llvm::Value* CodeGenerator::generateCode(NumberExprAST* expr) {
    return llvm::ConstantFP::get(*context, llvm::APFloat(expr->getValue()));
}

llvm::Value* CodeGenerator::generateCode(VariableExprAST* expr) {
    // Look this variable up in the function scope
    llvm::Value* V = namedValues[expr->getName()];
    if (!V) {
        reportError("Unknown variable name: " + expr->getName(), expr->getLocation());
        return nullptr;
    }
    return V;
}

llvm::Value* CodeGenerator::generateCode(BinaryExprAST* expr) {
    llvm::Value* L = generateCode(expr->getLHS());
    llvm::Value* R = generateCode(expr->getRHS());
    if (!L || !R) return nullptr;
    
    switch (expr->getOperator()) {
        case '+':
            return builder->CreateFAdd(L, R, "addtmp");
        case '-':
            return builder->CreateFSub(L, R, "subtmp");
        case '*':
            return builder->CreateFMul(L, R, "multmp");
        case '/':
            return builder->CreateFDiv(L, R, "divtmp");
        case '<':
            L = builder->CreateFCmpULT(L, R, "cmptmp");
            // Convert bool 0/1 to double 0.0 or 1.0
            return builder->CreateUIToFP(L, llvm::Type::getDoubleTy(*context), "booltmp");
        default:
            reportError("Invalid binary operator", expr->getLocation());
            return nullptr;
    }
}

llvm::Value* CodeGenerator::generateCode(CallExprAST* expr) {
    // Look up the name in the global module
    llvm::Function* calleeF = module->getFunction(expr->getCallee());
    if (!calleeF) {
        reportError("Unknown function: " + expr->getCallee(), expr->getLocation());
        return nullptr;
    }
    
    // Check argument count
    if (calleeF->arg_size() != expr->getArgs().size()) {
        reportError("Incorrect number of arguments to function", expr->getLocation());
        return nullptr;
    }
    
    std::vector<llvm::Value*> argsV;
    for (const auto& arg : expr->getArgs()) {
        argsV.push_back(generateCode(arg.get()));
        if (!argsV.back()) return nullptr;
    }
    
    return builder->CreateCall(calleeF, argsV, "calltmp");
}

llvm::Function* CodeGenerator::generateCode(PrototypeAST* proto) {
    // Get argument types
    std::vector<llvm::Type*> argTypes;
    for (unsigned i = 0; i < proto->getArgs().size(); ++i) {
        argTypes.push_back(llvm::Type::getDoubleTy(*context));
    }

    // Create function type
    llvm::FunctionType* FT = llvm::FunctionType::get(
        llvm::Type::getDoubleTy(*context), // Return type
        argTypes,                         // Argument types
        false);                           // Not varargs

    // Create the function with EXTERNAL linkage - this is critical!
    llvm::Function* F = llvm::Function::Create(
        FT,
        llvm::Function::ExternalLinkage,  // Must be external!
        proto->getName(),                 // Use actual function name
        module.get());

    // CRITICAL: Set the C calling convention
    F->setCallingConv(llvm::CallingConv::C);

    // Name arguments
    unsigned idx = 0;
    for (auto &arg : F->args()) {
        arg.setName(proto->getArgs()[idx++]);
    }

    return F;
}

llvm::Function* CodeGenerator::generateCode(FunctionAST* func) {
    // First, check for an existing function from a previous declaration
    llvm::Function* theFunction = module->getFunction(func->getPrototype()->getName());

    if (!theFunction) {
        theFunction = generateCode(func->getPrototype());
    }

    if (!theFunction) return nullptr;

    // Create a new basic block to start insertion into
    llvm::BasicBlock* BB = llvm::BasicBlock::Create(*context, "entry", theFunction);
    builder->SetInsertPoint(BB);

    // Record the function arguments in the namedValues map
    namedValues.clear();
    for (auto &arg : theFunction->args()) {
        namedValues[std::string(arg.getName())] = &arg;
    }

    // Generate the body code
    if (llvm::Value* retVal = generateCode(func->getBody())) {
        // Finish off the function
        builder->CreateRet(retVal);

        // Validate the generated code, checking for consistency
        llvm::verifyFunction(*theFunction);

        return theFunction;
    }

    // Error reading body, remove function
    theFunction->eraseFromParent();
    return nullptr;
}

llvm::Value* CodeGenerator::generateCode(StmtAST* stmt) {
    if (!stmt->getExpression()) return nullptr;
    return generateCode(stmt->getExpression());
}

bool CodeGenerator::generateCode(Program* program) {
    if (!program) return false;
    
    // First, generate all function prototypes to allow for forward references
    for (const auto& func : program->getFunctions()) {
        if (auto* proto = func->getPrototype()) {
            generateCode(proto);
        }
    }
    
    // Then generate all function bodies
    for (const auto& func : program->getFunctions()) {
        if (!generateCode(func.get())) {
            return false;
        }
        
        // If this is the main function, create a C-compatible wrapper for it
        if (func->getPrototype()->getName() == "main") {
            createMainWrapper();
        }
    }
    
    // Finally, generate top-level statements
    for (const auto& stmt : program->getStatements()) {
        if (!generateCode(stmt.get())) {
            return false;
        }
    }
    
    return !hasErrors();
}

// Add this new method to your CodeGenerator class
void CodeGenerator::createMainWrapper() {
    // Find the original main function
    llvm::Function* originalMain = module->getFunction("main");
    if (!originalMain) {
        return; // No main function to wrap
    }
    
    // Create a type for the wrapper function
    llvm::FunctionType* wrapperType = llvm::FunctionType::get(
        llvm::Type::getInt32Ty(*context), // Return int32 to match C main
        false // No parameters
    );
    
    // Create the wrapper function with explicit C linkage
    llvm::Function* wrapper = llvm::Function::Create(
        wrapperType,
        llvm::Function::ExternalLinkage,
        "tasia_main", // Use a unique name that won't conflict with C main
        module.get()
    );
    
    // Mark it with C linkage to prevent name mangling
    wrapper->setCallingConv(llvm::CallingConv::C);
    
    // Create a basic block for the wrapper function
    llvm::BasicBlock* block = llvm::BasicBlock::Create(*context, "entry", wrapper);
    builder->SetInsertPoint(block);
    
    // Call the original Tasia main function
    llvm::CallInst* result = builder->CreateCall(originalMain);
    
    // Convert the result to int32 (if needed) and return it
    llvm::Value* retVal;
    if (originalMain->getReturnType()->isDoubleTy()) {
        // Convert double to int32 by truncation
        retVal = builder->CreateFPToSI(result, llvm::Type::getInt32Ty(*context), "result_as_int");
    } else {
        retVal = result;
    }
    
    builder->CreateRet(retVal);
    
    // Verify the function is well-formed
    llvm::verifyFunction(*wrapper);
}