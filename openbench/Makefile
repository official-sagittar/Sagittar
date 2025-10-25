# ==========================================================
# Sagittar OpenBench Makefile  (Windows-safe)
# ==========================================================

EXE        ?= Sagittar
BIN_DIR    ?= .
ROOT_DIR   := $(realpath $(dir $(lastword $(MAKEFILE_LIST)))/..)
BUILD_ROOT := $(ROOT_DIR)/build
CXX        := clang++

# ----------------------------------------------------------
# Detect if running under Windows CMD / PowerShell
# ----------------------------------------------------------
ifeq ($(OS),Windows_NT)
	SHELL := cmd
	DEL_CMD := if exist "$(subst /,\,$(BIN_DIR))\\$(EXE).exe" del /Q "$(subst /,\,$(BIN_DIR))\\$(EXE).exe" & if exist "$(subst /,\,$(BIN_DIR))\\$(EXE)" del /Q "$(subst /,\,$(BIN_DIR))\\$(EXE)"
	RMDIR_CMD := if exist "$(subst /,\,$(BUILD_ROOT))" rmdir /S /Q "$(subst /,\,$(BUILD_ROOT))"
	MKDIR_CMD := if not exist "$(subst /,\,$(BUILD_ROOT))\\$(1)" mkdir "$(subst /,\,$(BUILD_ROOT))\\$(1)"
	IS_WINDOWS := 1
else
	DEL_CMD := rm -f "$(BIN_DIR)/$(EXE)" "$(BIN_DIR)/$(EXE).exe"
	RMDIR_CMD := rm -rf "$(BUILD_ROOT)"
	MKDIR_CMD := mkdir -p "$(BUILD_ROOT)/$(1)"
	IS_WINDOWS := 0
endif

# ----------------------------------------------------------
# Helper: configure + build via CMake
# ----------------------------------------------------------
define run_cmake
	@echo "=== Configuring $(1) build ==="
	@$(call MKDIR_CMD,$(1)) >nul 2>&1 || true
	cmake -S "$(ROOT_DIR)" -B "$(BUILD_ROOT)/$(1)" \
		-G "Ninja" \
		-DCMAKE_BUILD_TYPE=$(2) \
		-DENABLE_SAN=$(3) \
		-DEXE=$(EXE) \
		-DBIN_DIR=$(BIN_DIR) \
		-DCMAKE_CXX_COMPILER=$(CXX)
	@echo "=== Building $(EXE) ($(1)) ==="
	cmake --build "$(BUILD_ROOT)/$(1)" --target $(EXE)
	@echo "=== Build complete: $(BIN_DIR)/$(EXE) ==="
endef

.PHONY: all release clean info

all: clean release

release: ; $(call run_cmake,release,Release,OFF)

# ----------------------------------------------------------
# Clean target — pure Windows commands if under cmd.exe
# ----------------------------------------------------------
clean:
	@echo "=== Cleaning build artifacts ==="
ifeq ($(IS_WINDOWS),1)
	@$(RMDIR_CMD)
	@$(DEL_CMD)
else
	@$(RMDIR_CMD)
	@$(DEL_CMD)
endif

info:
	@echo "---------------------------------------------"
	@echo "Compiler   : $(CXX)"
	@echo "Executable : $(EXE)"
	@echo "Root Dir   : $(ROOT_DIR)"
	@echo "Build Root : $(BUILD_ROOT)"
	@echo "Binary Dir : $(BIN_DIR)"
	@echo "---------------------------------------------"
