# Detect operating system
OS := $(shell uname -s)
ARCH := $(shell uname -m)

# Compiler and paths
LLVM_PATH = /opt/homebrew/opt/llvm
CC = $(LLVM_PATH)/bin/clang++
LLVM_CONFIG = $(LLVM_PATH)/bin/llvm-config

# Installation directories
ifeq ($(OS),Darwin)
    # macOS
    INSTALL_DIR = /usr/local/bin
else ifeq ($(OS),Linux)
    # Linux
    INSTALL_DIR = /usr/bin
else
    # Windows or other
    INSTALL_DIR = /usr/bin
endif

# Source files
SOURCES := $(shell find . -name "*.cpp")

# Compiler flags
CXXFLAGS = -stdlib=libc++ -std=c++17 -g -O3
INCLUDES = -I$(LLVM_PATH)/include -I./
LLVMFLAGS = $(shell $(LLVM_CONFIG) --cxxflags --ldflags --system-libs --libs all)

# Target name (temporary build artifact)
TEMP_TARGET = _tasia_build_temp

.PHONY: all clean install uninstall

# Default target just shows help
all:
	@echo "To install the Tasia compiler system-wide, run:"
	@echo "    make install"
	@echo ""
	@echo "To remove the Tasia compiler, run:"
	@echo "    make uninstall"

# Build and immediately install
install: build_temp
	@echo "Installing to $(INSTALL_DIR)..."
	@sudo cp $(TEMP_TARGET) $(INSTALL_DIR)/tasia
	@sudo chmod 755 $(INSTALL_DIR)/tasia
	@rm -f $(TEMP_TARGET)
	@echo "Installation complete. You can now use 'tasia' directly from any directory."

# Temporary build target (not intended to be used directly)
build_temp:
	@echo "Building compiler..."
	$(CC) $(CXXFLAGS) $(INCLUDES) $(LLVMFLAGS) $(SOURCES) -o $(TEMP_TARGET)

uninstall:
	@echo "Removing from $(INSTALL_DIR)..."
	@sudo rm -f $(INSTALL_DIR)/tasia
	@echo "Uninstallation complete."

clean:
	@echo "Cleaning..."
	@rm -f $(TEMP_TARGET)
	@echo "Clean complete."