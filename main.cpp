#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <optional>
#include "lexer/lexer.h"
#include "parser/parser.h"
#include "codegen/CodeGenerator.h"
#include <llvm/Support/raw_ostream.h>
#include <llvm/Support/TargetSelect.h>
#include <llvm/MC/TargetRegistry.h>
#include <llvm/Target/TargetMachine.h>
#include <llvm/Target/TargetOptions.h>
#include <llvm/Support/FileSystem.h>
#include <llvm/IR/LegacyPassManager.h>
#include <llvm/TargetParser/Host.h> // For getDefaultTargetTriple in LLVM 19

std::string readFile(const std::string &path) {
    std::ifstream file(path);
    if (!file) {
        std::cerr << "Error: Could not open file " << path << std::endl;
        return "";
    }
    
    std::stringstream buffer;
    buffer << file.rdbuf();
    return buffer.str();
}

bool compileFile(const std::string &filepath, const std::string &outputPath) {
    // 1. Read the source file
    std::string source = readFile(filepath);
    if (source.empty()) {
        return false;
    }
    
    // 2. Create a lexer
    Lexer lexer(source);
    
    // 3. Create a parser with the lexer
    Parser parser(lexer, filepath);
    
    // 4. Parse the file
    auto program = parser.parseFile();
    
    // 5. Check for parsing errors
    if (parser.hasErrors()) {
        for (const auto &error : parser.getErrors()) {
            std::cerr << error.location.filename << ":" 
                     << error.location.line << ":" 
                     << error.location.column << ": error: " 
                     << error.message << std::endl;
        }
        return false;
    }
    
    // 6. Create a code generator
    CodeGenerator codeGen(filepath);
    
    // 7. Generate LLVM IR
    if (!codeGen.generateCode(program.get())) {
        for (const auto &error : codeGen.getErrors()) {
            std::cerr << error << std::endl;
        }
        return false;
    }
    
    // 8. Initialize LLVM targets
    llvm::InitializeAllTargetInfos();
    llvm::InitializeAllTargets();
    llvm::InitializeAllTargetMCs();
    llvm::InitializeAllAsmParsers();
    llvm::InitializeAllAsmPrinters();
    
    // 9. Get the target machine
    auto targetTriple = llvm::Triple::normalize(llvm::sys::getProcessTriple());
    std::string error;
    const llvm::Target* target = llvm::TargetRegistry::lookupTarget(targetTriple, error);
    
    if (!target) {
        std::cerr << "Error looking up target: " << error << std::endl;
        return false;
    }
    
    auto CPU = "generic";
    auto features = "";
    llvm::TargetOptions opt;
    std::optional<llvm::Reloc::Model> RM; // Using std::optional instead of llvm::Optional
    auto targetMachine = target->createTargetMachine(targetTriple, CPU, features, opt, RM);
    
    // 10. Configure the module for the target
    auto module = codeGen.getModule();
    module->setDataLayout(targetMachine->createDataLayout());
    module->setTargetTriple(targetTriple);
    
    // 11. Output the compiled code
    std::error_code EC;
    llvm::raw_fd_ostream dest(outputPath, EC, llvm::sys::fs::OF_None);
    
    if (EC) {
        std::cerr << "Could not open file: " << EC.message() << std::endl;
        return false;
    }
    
    // 12. Generate object code
    llvm::legacy::PassManager pass;
    
    // In LLVM 19, we use the enum directly rather than CGFT_ObjectFile
    // In main.cpp, change line 110 to:
    if (targetMachine->addPassesToEmitFile(pass, dest, nullptr, llvm::CodeGenFileType::ObjectFile)) {
        std::cerr << "Target machine can't emit a file of this type" << std::endl;
        return false;
    }
    
    pass.run(*module);
    dest.flush();
    
    std::cout << "Compiled " << filepath << " to " << outputPath << std::endl;
    return true;
}

int main(int argc, char** argv) {
    if (argc < 2) {
        std::cerr << "Usage: " << argv[0] << " <input_file.sia> [output_file.o]" << std::endl;
        return 1;
    }
    
    std::string inputFile = argv[1];
    std::string outputFile;
    
    if (argc >= 3) {
        outputFile = argv[2];
    } else {
        // Default output name: replace extension with .o
        outputFile = inputFile.substr(0, inputFile.find_last_of('.')) + ".o";
    }
    
    if (!compileFile(inputFile, outputFile)) {
        return 1;
    }
    
    return 0;
}