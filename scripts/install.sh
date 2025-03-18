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

# Check for LLVM and install if needed
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
    
    # Create or update the Makefile to use the right paths
    echo "Updating Makefile to use correct LLVM paths..."
    cd compiler
    cat > Makefile.local << EOF
# LLVM configuration specifically for this machine
LLVM_CFLAGS = -I${BREW_LLVM_PATH}/include
LLVM_LDFLAGS = -L${BREW_LLVM_PATH}/lib
LLVM_LIBS = -lLLVM
EOF
    
    # Build and install Tasia
    echo "Building and installing Tasia..."
    make clean
    make -f Makefile.local install
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