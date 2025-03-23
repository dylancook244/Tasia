#!/bin/bash
# Detect OS
OS="$(uname -s)"
echo "Installing Tasia for $OS..."

# Create a temporary directory for installation
TEMP_DIR=$(mktemp -d)
echo "Using temporary directory: $TEMP_DIR"
cd "$TEMP_DIR"

package_manager="none"

# Clone the repository
echo "Cloning the Tasia repository..."
git clone https://github.com/dylancook244/tasia.git
cd tasia

if [ -x "$(command -v apt)" ]; then
    package_manager="apt"
elif [ -x "$(command -v apt-get)" ]; then
    package_manager="apt-get"
elif [ -x "$(command -v dnf)" ]; then
    package_manager="dnf"
elif [ -x "$(command -v zypper)" ]; then
    package_manager="zypper"
elif [ -x "$(command -v brew)" ]; then
    package_manager="brew"
else
    echo "FAILED TO INSTALL PACKAGE: Package manager not found"
    exit 1
fi

echo "Using package manager: ${package_manager}"

# Navigate to the compiler directory
cd compiler

# Define source files (same for all platforms)
SOURCES="./parser/parser.c ./codegen/codegen.c ./ast/ast.c ./scanner/scanner.c ./scanner/token.c ./compile/compile.c ./compile/cli.c ./symbol_table/symbol_table.c main.c"

if [ "$OS" = "Darwin" ]; then
    # macOS
    # Check if LLVM is installed
    if $package_manager list llvm &>/dev/null; then
        echo "LLVM is already installed through Homebrew"
    else
        echo "Installing LLVM..."
        $package_manager install llvm
    fi
    
    # Get the actual LLVM path from Homebrew
    LLVM_PATH=$($package_manager --prefix llvm)
    echo "LLVM is installed at: $LLVM_PATH"
    
    echo "Building with correct LLVM paths..."
    echo "Compiling Tasia..."
    clang -std=c11 -g -O2 -I${LLVM_PATH}/include $SOURCES -o tasia -L${LLVM_PATH}/lib -lLLVM
    
elif [ "$OS" = "Linux" ]; then
    # Linux
    sudo $package_manager update
    sudo $package_manager install -y llvm llvm-dev clang
    
    echo "Building and installing Tasia..."
    # For Linux, we'll use pkg-config to get the proper LLVM flags
    LLVM_CFLAGS=$(pkg-config --cflags llvm)
    LLVM_LIBS=$(pkg-config --libs llvm)
    
    echo "Compiling Tasia..."
    clang -std=c11 -g -O2 ${LLVM_CFLAGS} $SOURCES -o tasia ${LLVM_LIBS}
fi

# Check if compilation succeeded
if [ $? -eq 0 ]; then
    echo "Compilation successful! Installing Tasia..."
    sudo mkdir -p /usr/local/bin
    sudo cp tasia /usr/local/bin/
    sudo chmod 755 /usr/local/bin/tasia
    echo "Installation complete!"
else
    echo "Compilation failed."
    exit 1
fi

# Clean up the repository
echo "Cleaning up temporary files..."
cd "$TEMP_DIR/.."
rm -rf "$TEMP_DIR"
echo "Tasia has been successfully installed!"
echo "You can now use the 'tasia' command to compile Tasia programs."