# Detect operating system
OS := $(shell uname -s 2>/dev/null || echo Windows)
ARCH := $(shell uname -m 2>/dev/null || echo x86_64)

# Set up paths based on OS
ifeq ($(OS),Darwin)
    # macOS
    LLVM_PATH ?= /opt/homebrew/opt/llvm
    INSTALL_DIR = /usr/local/bin
    SUDO = sudo
else ifeq ($(OS),Linux)
    # Linux - try to find LLVM using which command
    LLVM_CONFIG_PATH := $(shell which llvm-config 2>/dev/null)
    LLVM_PATH := $(if $(LLVM_CONFIG_PATH),$(dir $(shell dirname $(LLVM_CONFIG_PATH))),/usr/lib/llvm-19)
    INSTALL_DIR = /usr/bin
    SUDO = sudo
else
    # Windows
    LLVM_PATH ?= "C:/Program Files/LLVM"
    INSTALL_DIR = "C:/Program Files/tasia/bin"
    SUDO =
    # Remove extra quotes after substitution for Windows commands
    LLVM_PATH_CLEAN := $(subst ",,$(subst \,/,$(LLVM_PATH)))
endif

# Dynamic compiler detection
ifeq ($(OS),Windows)
    CC = "$(LLVM_PATH_CLEAN)/bin/clang++.exe"
    LLVM_CONFIG = "$(LLVM_PATH_CLEAN)/bin/llvm-config.exe"
    EXE_EXT = .exe
else
    CC = $(LLVM_PATH)/bin/clang++
    LLVM_CONFIG = $(LLVM_PATH)/bin/llvm-config
    EXE_EXT =
endif

# Source files (Windows-compatible)
ifeq ($(OS),Windows)
    SOURCES := $(shell dir /s /b *.cpp 2>NUL)
else
    SOURCES := $(shell find . -name "*.cpp")
endif

# Adjust flags for different platforms
CXXFLAGS = -std=c++17 -g -O3
ifeq ($(OS),Darwin)
    CXXFLAGS += -stdlib=libc++
endif

# Include paths with proper quoting
ifeq ($(OS),Windows)
    INCLUDES = -I"$(LLVM_PATH_CLEAN)/include" -I./
else
    INCLUDES = -I$(LLVM_PATH)/include -I./
endif

# Handle LLVM flags differently on Windows
ifeq ($(OS),Windows)
    # Manually specify LLVM libraries for Windows
    LLVMFLAGS = -L"$(LLVM_PATH_CLEAN)/lib" -lLLVMCore -lLLVMSupport -lLLVMX86CodeGen
else
    LLVMFLAGS = $(shell $(LLVM_CONFIG) --cxxflags --ldflags --system-libs --libs all)
endif

# Target name
TEMP_TARGET = _tasia_build_temp$(EXE_EXT)
FINAL_TARGET = tasia$(EXE_EXT)

# Rules
.PHONY: all clean install uninstall

all:
	@echo "To install the Tasia compiler, run:"
	@echo "    make install"
	@echo ""
	@echo "To remove the Tasia compiler, run:"
	@echo "    make uninstall"

install: build_temp
	@echo "Installing to $(INSTALL_DIR)..."
ifeq ($(OS),Windows)
	@mkdir "$(INSTALL_DIR)" 2>NUL || echo Directory exists
	@copy /Y "$(TEMP_TARGET)" "$(INSTALL_DIR)\$(FINAL_TARGET)"
else
	@$(SUDO) mkdir -p "$(INSTALL_DIR)"
	@$(SUDO) cp $(TEMP_TARGET) $(INSTALL_DIR)/$(FINAL_TARGET)
	@$(SUDO) chmod 755 $(INSTALL_DIR)/$(FINAL_TARGET)
endif
	@rm -f $(TEMP_TARGET) 2>NUL || del $(TEMP_TARGET) 2>NUL
	@echo "Installation complete. You can now use 'tasia' directly."

build_temp:
	@echo "Building compiler..."
	$(CC) $(CXXFLAGS) $(INCLUDES) $(LLVMFLAGS) $(SOURCES) -o $(TEMP_TARGET)

uninstall:
	@echo "Removing from $(INSTALL_DIR)..."
ifeq ($(OS),Windows)
	@del "$(INSTALL_DIR)\$(FINAL_TARGET)" 2>NUL || echo Already removed
else
	@$(SUDO) rm -f $(INSTALL_DIR)/$(FINAL_TARGET)
endif
	@echo "Uninstallation complete."

clean:
	@echo "Cleaning..."
	@rm -f $(TEMP_TARGET) 2>NUL || del $(TEMP_TARGET) 2>NUL
	@echo "Clean complete."