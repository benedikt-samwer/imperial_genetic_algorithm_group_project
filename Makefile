.PHONY: all build test performance format clean install-deps install-deps-mac install-deps-linux help

# Default target
all: build

# Detect OS
UNAME := $(shell uname -s)

# Set LLVM paths for macOS
ifeq ($(UNAME), Darwin)
    LLVM_PREFIX := /opt/homebrew/opt/llvm
    export PATH := $(LLVM_PREFIX)/bin:$(PATH)
    export LDFLAGS := -L$(LLVM_PREFIX)/lib
    export CPPFLAGS := -I$(LLVM_PREFIX)/include
endif

# Check for required tools
CLANG_FORMAT := $(shell command -v clang-format 2> /dev/null)
CLANG_TIDY := $(shell command -v clang-tidy 2> /dev/null)

# Help target
help:
	@echo "Available targets:"
	@echo "  make build        - Build the project"
	@echo "  make run          - Run the project"
	@echo "  make test         - Run all tests"
	@echo "  make performance  - Run performance tests"
	@echo "  make format       - Format code and run linter"
	@echo "  make clean        - Clean build directory"
	@echo "  make install-deps - Install dependencies (auto-detects OS)"
	@echo "  make help         - Show this help message"

# Build target
build:
	@bash build.sh

# Run target
run:
	@bash build.sh
	@bash run.sh

# Test target
test:
	@bash run_tests.sh

# Performance test target
performance:
	@bash testperformance.sh

# Format target
format:
	@echo "Checking for clang-format..."
ifndef CLANG_FORMAT
	$(error "clang-format is not installed. Please install it first")
endif
	@echo "Formatting all C/C++ files..."
	@find . -type f \( -name "*.cpp" -o -name "*.hpp" -o -name "*.c" -o -name "*.h" \) \
		-not -path "./build/*" \
		-not -path "./_deps/*" \
		-not -path "./Testing/*" \
		-exec clang-format -i -style=file {} +
	@echo "Code formatting completed!"

# Clean target
clean:
	@rm -rf build
	@echo "Cleaned build directory"

# Install dependencies for macOS
install-deps-mac:
	@echo "Installing dependencies for macOS..."
	@brew install cmake ninja gcc@14 llvm
	@brew link --force llvm
	@echo "Setting up LLVM paths..."
	@echo 'export PATH="$(LLVM_PREFIX)/bin:$$PATH"' >> ~/.zshrc
	@echo 'export LDFLAGS="-L$(LLVM_PREFIX)/lib"' >> ~/.zshrc
	@echo 'export CPPFLAGS="-I$(LLVM_PREFIX)/include"' >> ~/.zshrc
	@echo "Dependencies installed for macOS. Please restart your terminal or run:"
	@echo "   source ~/.zshrc"

# Install dependencies for Linux
install-deps-linux:
	@echo "Installing dependencies for Linux..."
	@sudo apt-get update
	@sudo apt-get install -y cmake ninja-build gcc g++ clang-format clang-tidy
	@echo "Dependencies installed for Linux"

# Auto-detect OS and install dependencies
install-deps:
ifeq ($(UNAME), Darwin)
	@$(MAKE) install-deps-mac
else
	@$(MAKE) install-deps-linux
endif

# Default to help if no target is specified
.DEFAULT_GOAL := help 