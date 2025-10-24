CXX       ?= clang++
EXE       ?= Sagittar
BIN_DIR   ?= .

ROOT_DIR  := $(realpath $(dir $(lastword $(MAKEFILE_LIST)))/..)
BUILD_DIR := $(ROOT_DIR)/build/openbench
TOOLCHAIN := $(ROOT_DIR)/cmake/clang-toolchain.cmake

UNAME_S := $(shell uname -s 2>/dev/null || echo Unknown)
ifeq ($(UNAME_S),Darwin)
    OS_NAME := macOS
else ifeq ($(findstring MINGW,$(UNAME_S)),MINGW)
    OS_NAME := Windows
else ifeq ($(findstring MSYS,$(UNAME_S)),MSYS)
    OS_NAME := Windows
else ifeq ($(findstring CYGWIN,$(UNAME_S)),CYGWIN)
    OS_NAME := Windows
else ifeq ($(UNAME_S),Linux)
    OS_NAME := Linux
else
    OS_NAME := Unknown
endif

ifeq ($(OS_NAME),Windows)
    NPROC := $(NUMBER_OF_PROCESSORS)
else
    NPROC := $(shell nproc 2>/dev/null || sysctl -n hw.ncpu 2>/dev/null || echo 4)
endif

.PHONY: all clean rebuild info

all: info
	@echo "=== Building $(EXE) with $(CXX) into $(BIN_DIR) on $(OS_NAME) ==="
	cmake -S $(ROOT_DIR) -B $(BUILD_DIR) \
		-DCMAKE_BUILD_TYPE=Release \
		-DCMAKE_TOOLCHAIN_FILE=$(TOOLCHAIN) \
		-DENABLE_SAN=OFF \
		-DEXE=$(EXE) \
		-DBIN_DIR=$(BIN_DIR) \
		-DCMAKE_CXX_COMPILER=$(CXX)
	cmake --build $(BUILD_DIR) --target $(EXE) -j $(NPROC)
	@echo "=== Build complete: $(BIN_DIR)/$(EXE) ==="

clean:
	@echo "=== Cleaning build artifacts ==="
ifeq ($(OS_NAME),Windows)
	@if exist "$(BUILD_DIR)" rmdir /S /Q "$(subst /,\,$(BUILD_DIR))"
	@if exist "$(BIN_DIR)/$(EXE).exe" del /Q "$(subst /,\,$(BIN_DIR))/$(EXE).exe"
else
	rm -rf $(BUILD_DIR) $(BIN_DIR)/$(EXE) $(BIN_DIR)/$(EXE).exe
endif

rebuild: clean all

info:
	@echo "Compiler   : $(CXX)"
	@echo "Executable : $(EXE)"
	@echo "Binary Dir : $(BIN_DIR)"
	@echo "Root Dir   : $(ROOT_DIR)"
	@echo "Toolchain  : $(TOOLCHAIN)"
	@echo "OS Detected: $(OS_NAME)"
	@echo "Cores Used : $(NPROC)"
