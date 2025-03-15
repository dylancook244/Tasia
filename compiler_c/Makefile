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
ifeq ($(OS),Windows)
    CC = clang.exe
    EXE_EXT = .exe
else
    CC = clang
    EXE_EXT =
endif

# Source files (Windows-compatible)
ifeq ($(OS),Windows)
    SOURCES := $(shell dir /s /b *.c 2>NUL)
else
    SOURCES := $(shell find . -name "*.c")
endif

# Compiler flags
CFLAGS = -std=c11 -g -O3

# Target name
TEMP_TARGET_NAME = _tasia_build_temp
TEMP_TARGET = $(TEMP_TARGET_NAME)$(EXE_EXT)
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
	@for /f "tokens=2,*" %%A in ('reg query "HKCU\Environment" /v PATH') do @setx PATH "$(INSTALL_DIR);%%B"
	@del $(TEMP_TARGET) 2>NUL
	@del $(TEMP_TARGET_NAME).ilk 2>NUL
	@del $(TEMP_TARGET_NAME).pdb 2>NUL
else
	@$(SUDO) mkdir -p "$(INSTALL_DIR)"
	@$(SUDO) cp $(TEMP_TARGET) $(INSTALL_DIR)/$(FINAL_TARGET)
	@$(SUDO) chmod 755 $(INSTALL_DIR)/$(FINAL_TARGET)
	@rm -f $(TEMP_TARGET) 2>NUL
endif
	@echo "Installation complete. You can now use 'tasia' directly."

build_temp:
	@echo "Building compiler..."
	@echo "Command: $(CC) $(CFLAGS) $(SOURCES) -o $(TEMP_TARGET)"
	$(CC) $(CFLAGS) $(SOURCES) -o $(TEMP_TARGET)

uninstall:
	@echo "Removing from $(INSTALL_DIR)..."
ifeq ($(OS),Windows)
	del "$(INSTALL_DIR)\$(FINAL_TARGET)" 2>NUL || echo Already removed
else
	@$(SUDO) rm -f $(INSTALL_DIR)/$(FINAL_TARGET)
endif
	@echo "Uninstallation complete."

clean:
	@echo "Cleaning..."
	@rm -f $(TEMP_TARGET) 2>NUL || del $(TEMP_TARGET) 2>NUL
	@echo "Clean complete."