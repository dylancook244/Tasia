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
if ! command -v llvm-config &> /dev/null; then
    echo "LLVM development files not found. Installing LLVM..."
    if [[ "$OS" == "Darwin" ]]; then
        # macOS
        brew install llvm
        export PATH="/usr/local/opt/llvm/bin:$PATH"
    elif [[ "$OS" == "Linux" ]]; then
        # Linux (Ubuntu/Debian assumed)
        sudo apt-get update
        sudo apt-get install -y llvm llvm-dev
    fi
fi

# Check for required C compiler
if ! command -v clang &> /dev/null; then
    echo "Clang not found. Installing Clang..."
    if [[ "$OS" == "Darwin" ]]; then
        # macOS - should already have clang via Xcode tools
        xcode-select --install
    elif [[ "$OS" == "Linux" ]]; then
        # Linux
        sudo apt-get update
        sudo apt-get install -y clang
    fi
fi

# Navigate to the compiler directory
echo "Navigating to compiler directory..."
cd compiler

# Build and install Tasia
echo "Building and installing Tasia..."
make install

# Clean up the repository
echo "Cleaning up temporary files..."
cd "$TEMP_DIR"
rm -rf tasia

echo "Tasia has been successfully installed!"
echo "You can now use the 'tasia' command to compile Tasia programs."