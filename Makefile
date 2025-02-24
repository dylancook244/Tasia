# Compiler and paths
LLVM_PATH = /opt/homebrew/opt/llvm
CC = $(LLVM_PATH)/bin/clang++
LLVM_CONFIG = $(LLVM_PATH)/bin/llvm-config

# Source files - we'll list them explicitly since the directories don't exist
SOURCES = lexer/lexer.cpp parser/parser.cpp codegen/CodeGenerator.cpp main.cpp

# Compiler flags
CXXFLAGS = -stdlib=libc++ -std=c++17 -g -O3
INCLUDES = -I$(LLVM_PATH)/include -I./
LLVMFLAGS = $(shell $(LLVM_CONFIG) --cxxflags --ldflags --system-libs --libs all)

# Our binary name - putting it in bin/ directory
TARGET = bin/Tasia

.PHONY: all clean

all: $(TARGET)

# Make sure the bin directory exists
$(TARGET): | bin

bin:
	mkdir -p bin

# Build everything directly into the binary, no intermediate files
$(TARGET): $(SOURCES)
	$(CC) $(CXXFLAGS) $(INCLUDES) $(LLVMFLAGS) $(SOURCES) -o $@

clean:
	rm -rf bin