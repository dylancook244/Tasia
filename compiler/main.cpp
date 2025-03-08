// general C programs
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <optional>
#include <filesystem>
#include <cstdlib>

// project files
#include "lexer/lexer.h"
#include "parser/parser.h"
#include "codegen/CodeGenerator.h"

// llvm files
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

bool compileFile(const std::string &filepath, const std::string &outputPath, const std::string &flagString) {
    // 1. Setup debug output if needed
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

    // 2. Read source
    std::string source = readFile(filepath);
    if (source.empty()) {
        if (debugFile.is_open()) {
            debugFile << "ERROR: Could not read source file: " << filepath << "\n";
            debugFile.close();
        }
        return false;
    }

    // if we need debug output, put the source code in we read, just raw sia
    if (debugFile.is_open()) {
        debugFile << "=== SOURCE CODE ===\n\n";
        
        // Add line numbers to make it easier to reference
        std::istringstream sourceStream(source);
        std::string line;
        int lineNum = 1;
        
        while (std::getline(sourceStream, line)) {
            debugFile << std::setw(4) << lineNum << " | " << line << "\n";
            lineNum++;
        }
        
        debugFile << "\n\n";
        
        // Reset the stream position for the lexer
        sourceStream = std::istringstream(source);
    }
    
    // 3. Tokenize and dump tokens if debugging
    if (debugFile.is_open()) {
        debugFile << "=== TOKEN DUMP ===\n\n";
        
        // Create a copy of the lexer for token dumping
        Lexer tokenLexer(source);
        int token;
        do {
            token = tokenLexer.getNextToken();
            // Convert token to string and write to debug file
            std::string tokenStr;
            switch (token) {
                case tok_eof: tokenStr = "EOF"; break;
                case tok_line_end: tokenStr = "LINE_END"; break;
                case tok_func: tokenStr = "FUNC"; break;
                case tok_extern: tokenStr = "EXTERN"; break;
                case tok_identifier: 
                    tokenStr = "IDENTIFIER(" + tokenLexer.getIdentifier() + ")"; 
                    break;
                case tok_number: 
                    tokenStr = "NUMBER(" + std::to_string(tokenLexer.getNumber()) + ")"; 
                    break;
                default:
                    if (token >= 32 && token <= 126) {
                        // Printable ASCII character
                        tokenStr = "CHAR('" + std::string(1, (char)token) + "')";
                    } else {
                        tokenStr = "UNKNOWN(" + std::to_string(token) + ")";
                    }
            }
            debugFile << tokenStr << "\n";
        } while (token != tok_eof);
        
        debugFile << "\n\n";
    }
    
    // 4. Create a lexer for actual parsing (only once)
    Lexer lexer(source);
    
    // 5. Create a parser with the lexer
    Parser parser(lexer, filepath);
    
    // 6. Parse the file ONCE
    auto program = parser.parseFile();
    
    // 7. Check for parsing errors
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
    
    // 8. Dump AST if debugging
    if (debugFile.is_open() && program) {
        debugFile << "=== AST DUMP ===\n\n";
        debugFile << program->toString() << "\n\n\n";
    }
    
    // 9. Create a code generator
    CodeGenerator codeGen(filepath);
    
    // 10. Generate LLVM IR
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
    
    // 11. Dump LLVM IR if debugging
    if (debugFile.is_open()) {
        debugFile << "=== LLVM IR DUMP ===\n\n";
        std::string irStr;
        llvm::raw_string_ostream irStream(irStr);
        codeGen.getModule()->print(irStream, nullptr);
        debugFile << irStr << "\n\n";
    }
    
    // 12. Initialize LLVM targets
    llvm::InitializeAllTargetInfos();
    llvm::InitializeAllTargets();
    llvm::InitializeAllTargetMCs();
    llvm::InitializeAllAsmParsers();
    llvm::InitializeAllAsmPrinters();
    
    // 13. Get the target machine
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
    std::optional<llvm::Reloc::Model> RM; // Using std::optional instead of llvm::Optional
    auto targetMachine = target->createTargetMachine(targetTriple, CPU, features, opt, RM);
    
    // 14. Configure the module for the target
    auto module = codeGen.getModule();
    module->setDataLayout(targetMachine->createDataLayout());
    module->setTargetTriple(targetTriple);
    
    // 15. Output the compiled code
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
    
    // 16. Generate object code
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
    
    // 17. Dump object file disassembly if debugging
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

// convoluted as fuck, but necessary. we make a runner.c file 
// so our program actually links this file to the object file
// created for our language. It also provides an entry point
// for our program. 
const char* RUNNER_C_CONTENT = R"(
#include <stdio.h>

// The "main" function for our Tasia program
// was renamed to TASIA_ENTRY_FUNCTION so it doesn't fuck up 
// the C program that's ALSO looking for a main function.
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
    // strict command check make sure args are there
    if (argc < 3) {
        std::cerr << "Usage: " << argv[0] << " <command> <input_file.sia>" << std::endl;
        std::cerr << "Commands: " << std::endl;
        std::cerr << "  build - compile your tasia file into an executable program" << std::endl;
        std::cerr << "  run - compile and run your tasia file" << std::endl;
        return 1;  // Added missing return statement
    }
    
    std::string command = argv[1];
    std::string inputFile = argv[2];
    bool flag = false;
    std::string flagString;

    // check if there's a flag from the user
    if (argc > 3) {
        flag = true;
        flagString = argv[3];
    }
    
    // verify you didn't fuck up the commands
    if (command != "build" && command != "run") {
        std::cerr << "Error: Unknown command '" << command << "'" << std::endl;
        std::cerr << "Valid commands: run, build" << std::endl;
        return 1;
    }
    
    // get our file paths for our base file and object file
    std::string baseName = inputFile.substr(0, inputFile.find_last_of('.'));
    std::string objFile = baseName + ".o";
    std::string exeFile = baseName;
    
    // Compile to object file
    if (!compileFile(inputFile, objFile, flagString)) {
        std::cerr << "Error: compilation failed" << std::endl;
        return 1;
    }
    
    // for both run and build an executable is created
    // create a temporary runner.c file that we'll delete when we're done
    // we need a runner.c so clang has something to link against
    std::string tempRunnerPath = "temp_runner_" + std::to_string(std::time(nullptr)) + ".c";
    
    if (!writeRunnerToTempFile(tempRunnerPath)) {
        return 1;
    }
    
    // build clang command to make executable
    std::string clangCmd = "clang " + tempRunnerPath + " " + objFile + " -o " + exeFile;
    
    std::cout << "Linking executable: " << exeFile << std::endl;  // Fixed std::cout
    int linkResult = system(clangCmd.c_str());
    
    // clean up temporary runner file and object file for tasia file
    std::filesystem::remove(tempRunnerPath);

    // After successful linking
    if (linkResult == 0) {
        // Clean up the object file
        std::filesystem::remove(objFile);
        
        // If run, then execute the executable that was built
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