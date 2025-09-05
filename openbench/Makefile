# Detect the platform
ifeq ($(OS),Windows_NT)
    # Windows settings
    EXE_EXT := .exe
    PATHSEP := \\
else
    # Linux/Unix settings
    EXE_EXT :=
    PATHSEP := /
endif

BIN_DIR := .
OBJ_DIR := ..$(PATHSEP)obj
SRC_DIR := ..$(PATHSEP)src
INC_DIR := -I$(SRC_DIR)

EXE ?= Sagittar$(EXE_EXT)
CXX ?= clang++
CFLAGS := -MD -MP -DNDEBUG -I$(SRC_DIR) -m64 -Werror -flto -O3 -Wall -Wextra -std=c++20 -march=native
LDFLAGS := -m64 -static -flto

all: clean dir sagittar

clean:
	@echo "==== Cleaning ===="
ifeq ($(OS),Windows_NT)
	@if exist "$(OBJ_DIR)" rmdir /S /Q "$(OBJ_DIR)"
	@if exist "$(BIN_DIR)$(PATHSEP)$(EXE)" del /Q "$(BIN_DIR)$(PATHSEP)$(EXE)"
else
	@rm -rf $(OBJ_DIR)
	@rm -f $(BIN_DIR)$(PATHSEP)$(EXE)
endif

dir:
	@echo "==== Creating Required Folders ===="
ifeq ($(OS),Windows_NT)
	@if not exist "$(OBJ_DIR)" mkdir "$(OBJ_DIR)"
else
	@mkdir -p $(OBJ_DIR)
endif

sagittar:
	@echo "==== Building Sagittar ===="

.PHONY: clean dir
