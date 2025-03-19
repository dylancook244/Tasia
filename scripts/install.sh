#!/bin/bash

# Detect OS
OS="$(uname -s)"
echo "Installing Tasia for $OS..."

# Create a temporary directory for installation
TEMP_DIR=$(mktemp -d)
echo "Using temporary directory: $TEMP_DIR"
cd "$TEMP_DIR"

# Clone the repository
echo "Cloning the Tasia repository..."
git clone https://github.com/dylancook244/tasia.git
cd tasia

if [[ "$OS" == "Darwin" ]]; then
    # macOS
    # Check if LLVM is installed
    if brew list llvm &>/dev/null; then
        echo "LLVM is already installed through Homebrew"
    else
        echo "Installing LLVM through Homebrew..."
        brew install llvm
    fi
    
    # Get the actual LLVM path from Homebrew
    BREW_LLVM_PATH=$(brew --prefix llvm)
    echo "LLVM is installed at: $BREW_LLVM_PATH"
    
    # Navigate to the compiler directory
    cd compiler
    
    echo "Building with correct LLVM paths..."
    
    # Direct compilation command to avoid Makefile issues
    SOURCES="./symbol_table/symbol_table.c ./parser/parser.c ./codegen/codegen.c ./main.c ./ast/ast.c ./scanner/scanner.c ./scanner/token.c"
    
    echo "Compiling Tasia..."
    clang -std=c11 -g -O2 -I${BREW_LLVM_PATH}/include $SOURCES -o tasia -L${BREW_LLVM_PATH}/lib -lLLVM
    
    if [ $? == 0 ]; then
        echo "Compilation successful! Installing Tasia..."
        sudo mkdir -p /usr/local/bin
        sudo cp tasia /usr/local/bin/
        sudo chmod 755 /usr/local/bin/tasia
        echo "Installation complete!"
    else
        echo "Compilation failed."
        exit 1
    fi
    
elif [[ "$OS" == "Linux" ]]; then
    # Linux (Ubuntu/Debian assumed)
    sudo apt-get update
    sudo apt-get install -y llvm llvm-dev clang
    
    # Navigate to the compiler directory
    cd compiler
    
    # Build and install Tasia
    echo "Building and installing Tasia..."
    make clean
    make install
fi

# Clean up the repository
echo "Cleaning up temporary files..."
cd "$TEMP_DIR"
rm -rf tasia

echo "Tasia has been successfully installed!"
echo "You can now use the 'tasia' command to compile Tasia programs."