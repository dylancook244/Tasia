#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <optional>
#include <filesystem>
#include <cstdlib>

// Lexer and parser headers
#include "lexer/lexer.h"
#include "lexer/token.h"
#include "parser/parser.h"

// Symbol table and type system
#include "types/SymbolTable.h"
#include "types/Types.h"
#include "types/TypeChecker.h"
#include "types/BorrowChecker.h"

// Code generation
#include "codegen/CodeGenerator.h"

// LLVM includes
#include <llvm/Support/raw_ostream.h>
#include <llvm/Support/TargetSelect.h>
#include <llvm/MC/TargetRegistry.h>
#include <llvm/Target/TargetMachine.h>
#include <llvm/Target/TargetOptions.h>
#include <llvm/Support/FileSystem.h>
#include <llvm/IR/LegacyPassManager.h>
#include <llvm/TargetParser/Host.h>

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

bool compileFile(const std::string &filepath, const std::string &outputPath, const std::string &flagString) {
    // Setup debug output if needed
    std::ofstream debugFile;
    if (flagString == "--compilerOutput") {
        std::string debugPath = filepath.substr(0, filepath.find_last_of('.')) + ".debug.txt";
        debugFile.open(debugPath);
        if (!debugFile) {
            std::cerr << "Warning: Could not create debug output file" << std::endl;
        } else {
            debugFile << "=== TASIA COMPILER DEBUG OUTPUT ===\n\n";
            debugFile << "Source file: " << filepath << "\n";
            debugFile << "Output file: " << outputPath << "\n\n\n";
        }
    }

    // Read source
    std::string source = readFile(filepath);
    if (source.empty()) {
        if (debugFile.is_open()) {
            debugFile << "ERROR: Could not read source file: " << filepath << "\n";
            debugFile.close();
        }
        return false;
    }

    // Output source code with line numbers in debug
    if (debugFile.is_open()) {
        debugFile << "=== SOURCE CODE ===\n\n";
        
        std::istringstream sourceStream(source);
        std::string line;
        int lineNum = 1;
        
        while (std::getline(sourceStream, line)) {
            debugFile << std::setw(4) << lineNum << " | " << line << "\n";
            lineNum++;
        }
        
        debugFile << "\n\n";
    }
    
    // Token dump for debugging
    if (debugFile.is_open()) {
        debugFile << "=== TOKEN DUMP ===\n\n";
        
        Lexer tokenLexer(source);
        int token;
        do {
            token = tokenLexer.getNextToken();
            std::string tokenStr;
            switch (token) {
                case END_OF_FILE: tokenStr = "END_OF_FILE"; break;
                case LINE_END: tokenStr = "LINE_END"; break;
                case FUNC: tokenStr = "FUNC"; break;
                case IDENT: 
                    tokenStr = "IDENT(" + tokenLexer.getIdentifier() + ")"; 
                    break;
                case INT: 
                    tokenStr = "INT(" + std::to_string(tokenLexer.getNumber()) + ")"; 
                    break;
                // Add new token types
                case MUT: tokenStr = "MUT"; break;
                case CONST: tokenStr = "CONST"; break;
                case REF: tokenStr = "REF"; break;
                default:
                    if (token >= 32 && token <= 126) {
                        tokenStr = "CHAR('" + std::string(1, (char)token) + "')";
                    } else {
                        tokenStr = "UNKNOWN(" + std::to_string(token) + ")";
                    }
            }
            debugFile << tokenStr << "\n";
        } while (token != END_OF_FILE);
        
        debugFile << "\n\n";
    }
    
    // Parse the file
    Lexer lexer(source);
    Parser parser(lexer, filepath);
    auto program = parser.parseFile();
    
    // Check for parsing errors
    if (parser.hasErrors()) {
        for (const auto &error : parser.getErrors()) {
            std::cerr << error.location.filename << ":" 
                     << error.location.line << ":" 
                     << error.location.column << ": error: " 
                     << error.message << std::endl;
            
            if (debugFile.is_open()) {
                debugFile << "PARSE ERROR: " << error.location.filename << ":" 
                         << error.location.line << ":" << error.location.column 
                         << ": " << error.message << "\n";
            }
        }
        if (debugFile.is_open()) debugFile.close();
        return false;
    }
    
    // Dump AST in debug mode
    if (debugFile.is_open() && program) {
        debugFile << "=== AST DUMP ===\n\n";
        debugFile << program->toString() << "\n\n\n";
    }
    
    // Type checking and borrow checking
    TypeChecker typeChecker;
    if (!typeChecker.checkProgram(program.get())) {
        for (const auto &error : typeChecker.getErrors()) {
            std::cerr << error << std::endl;
            if (debugFile.is_open()) {
                debugFile << "TYPE/BORROW CHECK ERROR: " << error << "\n";
            }
        }
        if (debugFile.is_open()) debugFile.close();
        return false;
    }
    
    // Generate code
    CodeGenerator codeGen(filepath);
    if (!codeGen.generateCode(program.get())) {
        for (const auto &error : codeGen.getErrors()) {
            std::cerr << error << std::endl;
            if (debugFile.is_open()) {
                debugFile << "CODEGEN ERROR: " << error << "\n";
            }
        }
        if (debugFile.is_open()) debugFile.close();
        return false;
    }
    
    // Dump LLVM IR in debug mode
    if (debugFile.is_open()) {
        debugFile << "=== LLVM IR DUMP ===\n\n";
        std::string irStr;
        llvm::raw_string_ostream irStream(irStr);
        codeGen.getModule()->print(irStream, nullptr);
        debugFile << irStr << "\n\n";
    }
    
    // Initialize LLVM targets
    llvm::InitializeAllTargetInfos();
    llvm::InitializeAllTargets();
    llvm::InitializeAllTargetMCs();
    llvm::InitializeAllAsmParsers();
    llvm::InitializeAllAsmPrinters();
    
    // Get the target machine
    auto targetTriple = llvm::Triple::normalize(llvm::sys::getProcessTriple());
    std::string error;
    const llvm::Target* target = llvm::TargetRegistry::lookupTarget(targetTriple, error);
    
    if (!target) {
        std::cerr << "Error looking up target: " << error << std::endl;
        if (debugFile.is_open()) {
            debugFile << "TARGET ERROR: " << error << "\n";
            debugFile.close();
        }
        return false;
    }
    
    auto CPU = "generic";
    auto features = "";
    llvm::TargetOptions opt;
    std::optional<llvm::Reloc::Model> RM;
    auto targetMachine = target->createTargetMachine(targetTriple, CPU, features, opt, RM);
    
    // Configure module for target
    auto module = codeGen.getModule();
    module->setDataLayout(targetMachine->createDataLayout());
    module->setTargetTriple(targetTriple);
    
    // Output compiled code
    std::error_code EC;
    llvm::raw_fd_ostream dest(outputPath, EC, llvm::sys::fs::OF_None);
    
    if (EC) {
        std::cerr << "Could not open file: " << EC.message() << std::endl;
        if (debugFile.is_open()) {
            debugFile << "OUTPUT ERROR: Could not open output file: " << EC.message() << "\n";
            debugFile.close();
        }
        return false;
    }
    
    // Generate object code
    llvm::legacy::PassManager pass;
    
    if (targetMachine->addPassesToEmitFile(pass, dest, nullptr, llvm::CodeGenFileType::ObjectFile)) {
        std::cerr << "Target machine can't emit a file of this type" << std::endl;
        if (debugFile.is_open()) {
            debugFile << "EMISSION ERROR: Target machine can't emit a file of this type\n";
            debugFile.close();
        }
        return false;
    }
    
    pass.run(*module);
    dest.flush();
    
    // Dump object code disassembly in debug mode
    if (debugFile.is_open()) {
        debugFile << "=== OBJECT CODE DUMP ===\n";
        std::string objdumpCmd = "objdump -d " + outputPath;
        FILE* pipe = popen(objdumpCmd.c_str(), "r");
        if (pipe) {
            char buffer[128];
            while (fgets(buffer, sizeof(buffer), pipe) != NULL) {
                debugFile << buffer;
            }
            pclose(pipe);
        } else {
            debugFile << "Failed to run objdump command\n";
        }
    }
    
    // Close debug file
    if (debugFile.is_open()) {
        debugFile.close();
        std::cout << "Debug information written to: " 
                 << filepath.substr(0, filepath.find_last_of('.')) + ".debug.txt" << std::endl;
    }
    
    std::cout << "Compiled " << filepath << " to " << outputPath << std::endl;
    return true;
}

// Runner C content for linking
const char* RUNNER_C_CONTENT = R"(
#include <stdio.h>

// The "main" function for our Tasia program
extern double TASIA_ENTRY_FUNCTION(void);

int main(void) {
    double result = TASIA_ENTRY_FUNCTION();
    printf("Result: %.1f\n", result);
    return 0;
})";

bool writeRunnerToTempFile(const std::string& tempPath) {
    std::ofstream runnerFile(tempPath);
    if (!runnerFile) {
        std::cerr << "Error: couldn't create temporary runner file" << std::endl;
        return false;
    }

    runnerFile << RUNNER_C_CONTENT;
    runnerFile.close();
    return true;
}

int main(int argc, char** argv) {
    if (argc < 3) {
        std::cerr << "Usage: " << argv[0] << " <command> <input_file.sia>" << std::endl;
        std::cerr << "Commands: " << std::endl;
        std::cerr << "  build - compile your tasia file into an executable program" << std::endl;
        std::cerr << "  run - compile and run your tasia file" << std::endl;
        return 1;
    }
    
    std::string command = argv[1];
    std::string inputFile = argv[2];
    bool flag = false;
    std::string flagString;

    if (argc > 3) {
        flag = true;
        flagString = argv[3];
    }
    
    if (command != "build" && command != "run") {
        std::cerr << "Error: Unknown command '" << command << "'" << std::endl;
        std::cerr << "Valid commands: run, build" << std::endl;
        return 1;
    }
    
    std::string baseName = inputFile.substr(0, inputFile.find_last_of('.'));
    std::string objFile = baseName + ".o";
    std::string exeFile = baseName;
    
    // Compile to object file
    if (!compileFile(inputFile, objFile, flagString)) {
        std::cerr << "Error: compilation failed" << std::endl;
        return 1;
    }
    
    // Create temp runner.c file
    std::string tempRunnerPath = "temp_runner_" + std::to_string(std::time(nullptr)) + ".c";
    
    if (!writeRunnerToTempFile(tempRunnerPath)) {
        return 1;
    }
    
    // Build executable
    std::string clangCmd = "clang " + tempRunnerPath + " " + objFile + " -o " + exeFile;
    
    std::cout << "Linking executable: " << exeFile << std::endl;
    int linkResult = system(clangCmd.c_str());
    
    // Clean up temp files
    std::filesystem::remove(tempRunnerPath);

    if (linkResult == 0) {
        // Clean up the object file
        std::filesystem::remove(objFile);
        
        // Run if requested
        if (command == "run") {
            std::cout << "Running: " << exeFile << "..." << std::endl;
            int runResult = system(("./" + exeFile).c_str());
            return runResult;
        }
    } else {
        std::cerr << "Error: linking failed" << std::endl;
        return 1;
    }

    return 0;
}