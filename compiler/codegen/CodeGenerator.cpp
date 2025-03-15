#include "CodeGenerator.h"

#include <iostream>
#include <sstream>

void CodeGenerator::reportError(const std::string& msg, const SourceLocation& loc) {
  std::stringstream ss;
  ss << loc.filename << ":" << loc.line << ":" << loc.column
     << ": error: " << msg;
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
  } else if (auto* blockExpr = dynamic_cast<BlockExprAST*>(expr)) {
    return generateCode(blockExpr);
  }

  reportError("Unknown expression type", expr->getLocation());
  return nullptr;
}

// And implement it in CodeGenerator.cpp:
llvm::Value* CodeGenerator::generateCode(BlockExprAST *expr) {
  llvm::Value* lastVal = nullptr;

  // Generate code for each expression in the block
  for (const auto& exprPtr : expr->getExpressions()) {
    lastVal = generateCode(exprPtr.get());
    if (!lastVal) {
      return nullptr;  // Error occurred
    }
  }

  // The value of a block is the value of its last expression
  // If the block is empty, return a default value (like 0.0)
  if (!lastVal) {
    return llvm::ConstantFP::get(*context, llvm::APFloat(0.0));
  }

  return lastVal;
}

llvm::Value *CodeGenerator::generateCode(NumberExprAST *expr)
{
  return llvm::ConstantFP::get(*context, llvm::APFloat(expr->getValue()));
}

llvm::Value *CodeGenerator::generateCode(VariableExprAST *expr) {
  // Look this variable up in the function scope
  llvm::Value* V = namedValues[expr->getName()];
  if (!V) {
    reportError("Unknown variable name: " + expr->getName(),
                expr->getLocation());
    return nullptr;
  }
  return V;
}

llvm::Value *CodeGenerator::generateCode(BinaryExprAST *expr) {
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
      return builder->CreateUIToFP(L, llvm::Type::getDoubleTy(*context),
                                   "booltmp");
    default:
      reportError("Invalid binary operator", expr->getLocation());
      return nullptr;
  }
}

llvm::Value *CodeGenerator::generateCode(CallExprAST *expr) {
  // Look up the name in the global module
  llvm::Function* calleeF = module->getFunction(expr->getCallee());
  if (!calleeF) {
    reportError("Unknown function: " + expr->getCallee(), expr->getLocation());
    return nullptr;
  }

  // Check argument count
  if (calleeF->arg_size() != expr->getArgs().size()) {
    reportError("Incorrect number of arguments to function",
                expr->getLocation());
    return nullptr;
  }

  std::vector<llvm::Value*> argsV;
  for (const auto& arg : expr->getArgs()) {
    argsV.push_back(generateCode(arg.get()));
    if (!argsV.back()) return nullptr;
  }

  return builder->CreateCall(calleeF, argsV, "calltmp");
}

// this is for function bodies, their name, parameters, and returns
llvm::Function *CodeGenerator::generateCode(FuncInterfaceAST *funcInterface) {
  // get function name, with special handling for main
  std::string funcName = funcInterface->getName();

  // Get argument types
  std::vector<llvm::Type*> argTypes;
  for (unsigned i = 0; i < funcInterface->getArgs().size(); ++i) {
    argTypes.push_back(llvm::Type::getDoubleTy(*context));
  }

  // Create function type
  llvm::FunctionType* FT =
      llvm::FunctionType::get(llvm::Type::getDoubleTy(*context),  // Return type
        argTypes,  // Argument types
          =false);    // Not varargs

  // Create the function with EXTERNAL linkage - this is critical!
  llvm::Function* F = llvm::Function::Create(
      FT,
      llvm::Function::ExternalLinkage,  // Must be external!
      funcName, module.get());

  // CRITICAL: Set the C calling convention
  F->setCallingConv(llvm::CallingConv::C);

  // Name arguments
  unsigned idx = 0;
  for (auto& arg : F->args()) {
    arg.setName(funcInterface->getArgs()[idx++]);
  }

  return F;
}

llvm::Function *CodeGenerator::generateCode(FuncAST *func) {
  // First, check for an existing function from a previous declaration
  llvm::Function* theFunction =
      module->getFunction(func->getFuncInterface()->getName());

  if (!theFunction) {
    theFunction = generateCode(func->getFuncInterface());
  }

  if (!theFunction) return nullptr;

  // Create a new basic block to start insertion into
  llvm::BasicBlock* BB =
      llvm::BasicBlock::Create(*context, "entry", theFunction);
  builder->SetInsertPoint(BB);

  // Record the function arguments in the namedValues map
  namedValues.clear();
  for (auto& arg : theFunction->args()) {
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

llvm::Value *CodeGenerator::generateCode(StmtAST *stmt) {
  if (!stmt->getExpression()) return nullptr;
  return generateCode(stmt->getExpression());
}

bool CodeGenerator::generateCode(Program *program) {
  if (!program) return false;

  // First, generate all function interfaces to allow for forward references
  for (const auto& func : program->getFunctions()) {
    if (auto* funcInterface = func->getFuncInterface()) {
      generateCode(funcInterface);
    }
  }

  // Then generate all function bodies
  for (const auto& func : program->getFunctions()) {
    if (!generateCode(func.get())) {
      return false;
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

llvm::Value* CodeGenerator::generateCode(DeclarationExprAST* expr) {
  // Generate code for initialization if present
  llvm::Value* initVal = nullptr;
  if (expr->getInitExpr()) {
      initVal = generateCode(expr->getInitExpr());
      if (!initVal) return nullptr;
  } else {
      // Default initialization
      initVal = llvm::ConstantFP::get(*context, llvm::APFloat(0.0));
;
  }
  
  // Create an alloca for this variable
  llvm::Function* function = builder->GetInsertBlock()->getParent();
  llvm::AllocaInst* alloca = CreateEntryBlockAlloca(function, expr->getName());
  
  // Store the initial value into the alloca
  builder->CreateStore(initVal, alloca);
  
  // Remember this binding
  namedValues[expr->getName()] = alloca;
  
  return initVal;
}

llvm::Value* CodeGenerator::generateCode(ReferenceExprAST* expr) {
  // If we're taking a reference to a variable, simply return its address
  if (auto* varExpr = dynamic_cast<VariableExprAST*>(expr->getTarget())) {
      auto it = namedValues.find(varExpr->getName());
      if (it == namedValues.end()) {
          return logErrorV("Unknown variable name: " + varExpr->getName());
      }
      
      // For references, we just return the pointer
      return it->second;
  }
  
  // Otherwise, evaluate the expression and store it in a temporary
  llvm::Value* val = generateCode(expr->getTarget());
  if (!val) return nullptr;
  
  // Create a temporary to hold the value
  llvm::Function* function = builder->GetInsertBlock()->getParent();
  llvm::AllocaInst* tempAlloca = CreateEntryBlockAlloca(function, "ref.temp");
  
  // Store the value in the temporary
  builder->CreateStore(val, tempAlloca);
  
  // Return the address of the temporary
  return tempAlloca;
}

llvm::Value* CodeGenerator::generateCode(DereferenceExprAST* expr) {
  // Generate code for the reference
  llvm::Value* ref = generateCode(expr->getTarget());
  if (!ref) return nullptr;
  
  // If it's not a pointer type, error
  if (!ref->getType()->isPointerTy()) {
      return logErrorV("Cannot dereference non-pointer type");
  }
  
  if (ref->getType()->isPointerTy()) {
    // In LLVM 19, we need to use a different approach
    // Use double as the default pointee type since Tasia currently uses doubles for numbers
    llvm::Type *elementType = llvm::Type::getDoubleTy(*context);
    return builder->CreateLoad(elementType, ref, "deref");
  } else {
    reportError("Cannot dereference non-pointer type", expr->getLocation());
    return nullptr;
  }

}

llvm::AllocaInst* CodeGenerator::CreateEntryBlockAlloca(llvm::Function* function, const std::string& varName) {
  llvm::IRBuilder<> TmpB(&function->getEntryBlock(), function->getEntryBlock().begin());
  return TmpB.CreateAlloca(llvm::Type::getDoubleTy(*context), 0, varName);
}

llvm::Value* CodeGenerator::logErrorV(const std::string& str) {
  reportError(str, SourceLocation());
  return nullptr;
}