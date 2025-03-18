# Detect operating system
OS := $(shell uname -s 2>/dev/null || echo Windows)
ARCH := $(shell uname -m 2>/dev/null || echo x86_64)

# Set up paths based on OS
ifeq ($(OS),Darwin)
    # macOS
    INSTALL_DIR = /usr/local/bin
    SUDO = sudo
else ifeq ($(OS),Linux)
    # Linux
    INSTALL_DIR = /usr/bin
    SUDO = sudo
else
    # Windows
    INSTALL_DIR = C:/Program Files/tasia/bin
    SUDO =
endif

# Dynamic compiler detection
CC = clang
ifeq ($(OS),Windows)
    EXE_EXT = .exe
else
    EXE_EXT =
endif

# LLVM configuration
ifeq ($(OS),Darwin)
    # macOS - homebrew typically installs LLVM in a non-standard location
    LLVM_CONFIG = $(shell which llvm-config 2>/dev/null || echo /usr/local/opt/llvm/bin/llvm-config)
else ifeq ($(OS),Linux)
    LLVM_CONFIG = llvm-config
else
    # Windows
    LLVM_CONFIG = llvm-config
endif

# Get compiler and linker flags from llvm-config
LLVM_CFLAGS = $(shell $(LLVM_CONFIG) --cflags 2>/dev/null || echo "-I/usr/local/include")
LLVM_LDFLAGS = $(shell $(LLVM_CONFIG) --ldflags --system-libs 2>/dev/null || echo "-L/usr/local/lib")
LLVM_LIBS = $(shell $(LLVM_CONFIG) --libs core analysis bitwriter 2>/dev/null || echo "-lLLVM")

# Source files
ifeq ($(OS),Windows)
    SOURCES := $(shell dir /s /b *.c 2>NUL)
else
    SOURCES := $(shell find . -name "*.c")
endif

# Compiler flags - add -static for fully static builds if desired
CFLAGS = -std=c11 -g -O2 $(LLVM_CFLAGS)

# Target name
TARGET = tasia$(EXE_EXT)

# Rules
.PHONY: all clean install uninstall

all: $(TARGET)

$(TARGET): $(SOURCES)
	@echo "Building Tasia compiler..."
	@echo "Command: $(CC) $(CFLAGS) $(SOURCES) -o $(TARGET) $(LLVM_LDFLAGS) $(LLVM_LIBS)"
	$(CC) $(CFLAGS) $(SOURCES) -o $(TARGET) $(LLVM_LDFLAGS) $(LLVM_LIBS)

install: $(TARGET)
	@echo "Installing to $(INSTALL_DIR)..."
ifeq ($(OS),Windows)
	@mkdir "$(INSTALL_DIR)" 2>NUL || echo Directory exists
	@copy /Y "$(TARGET)" "$(INSTALL_DIR)\"
else
	$(SUDO) mkdir -p "$(INSTALL_DIR)"
	$(SUDO) cp $(TARGET) $(INSTALL_DIR)/
	$(SUDO) chmod 755 $(INSTALL_DIR)/$(TARGET)
endif
	@echo "Installation complete. You can now use 'tasia' directly."

uninstall:
	@echo "Removing from $(INSTALL_DIR)..."
ifeq ($(OS),Windows)
	del "$(INSTALL_DIR)\$(TARGET)" 2>NUL || echo Already removed
else
	$(SUDO) rm -f $(INSTALL_DIR)/$(TARGET)
endif
	@echo "Uninstallation complete."

clean:
	@echo "Cleaning..."
ifeq ($(OS),Windows)
	@if exist $(TARGET) del $(TARGET)
else
	@rm -f $(TARGET)
endif
	@echo "Clean complete."