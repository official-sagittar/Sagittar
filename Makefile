# Wrapper Makefile for OpenBench + CMake + PGO
# Works on Linux/macOS and Windows (cmd.exe / PowerShell)

EXE     ?= Sagittar
CXX     ?= clang++
CONFIG  ?= Release
BUILD_DIR := build
PGO_DIR := pgo-data

.DEFAULT_GOAL := all   # default target when running just "make"

.PHONY: all release debug testbuild sanity clean pgo-gen pgo-use pgo-auto help

# ===========================
# Cross-platform shell helpers
# ===========================
ifeq ($(OS),Windows_NT)
    MKDIR = if not exist $(1) mkdir $(1)
    RMDIR = if exist $(1) rmdir /s /q $(1)
    RMFILE = if exist $(1) del /q $(1)
    SEP = \\
    FIND_CLANG = where $(CXX) >nul 2>nul
else
    MKDIR = mkdir -p $(1)
    RMDIR = rm -rf $(1)
    RMFILE = rm -f $(1)
    SEP = /
    FIND_CLANG = which $(CXX) >/dev/null 2>&1
endif

# ===========================
# Safety check for compiler
# ===========================
check-compiler:
	@$(FIND_CLANG) || (echo "ERROR: $(CXX) not found in PATH. Please install LLVM/Clang and ensure it's available." && exit 1)

# ===========================
# Default build
# ===========================
all: check-compiler
	$(call MKDIR,$(BUILD_DIR))
	cmake -S . -B $(BUILD_DIR) \
	      -DCMAKE_BUILD_TYPE=$(CONFIG) \
	      -DCMAKE_TOOLCHAIN_FILE=clang-toolchain.cmake \
	      -DEXE=$(EXE) -DCMAKE_CXX_COMPILER=$(CXX)
ifeq ($(CONFIG),Test)
	cmake --build $(BUILD_DIR) --target $(EXE)_test -j -- VERBOSE=$(VERBOSE)
	@echo Built test binary: .$(SEP)$(EXE)_test
else ifeq ($(CONFIG),Sanity)
	cmake --build $(BUILD_DIR) --target $(EXE)_sanity -j -- VERBOSE=$(VERBOSE)
	@echo Built sanity binary: .$(SEP)$(EXE)_sanity
else
	cmake --build $(BUILD_DIR) --target $(EXE) -j -- VERBOSE=$(VERBOSE)
	@echo Built engine binary: .$(SEP)$(EXE)
endif

# ===========================
# Shortcuts
# ===========================
release:
	@$(MAKE) all CONFIG=Release VERBOSE=$(VERBOSE)

debug:
	@$(MAKE) all CONFIG=Debug VERBOSE=$(VERBOSE)

testbuild:
	@$(MAKE) all CONFIG=Test VERBOSE=$(VERBOSE)

sanity:
	@$(MAKE) all CONFIG=Sanity VERBOSE=$(VERBOSE)

# ===========================
# PGO
# ===========================
pgo-gen: check-compiler
	$(call MKDIR,$(BUILD_DIR))
	$(call MKDIR,$(PGO_DIR))
	cmake -S . -B $(BUILD_DIR) \
	      -DCMAKE_BUILD_TYPE=Release \
	      -DCMAKE_TOOLCHAIN_FILE=clang-toolchain.cmake \
	      -DEXE=$(EXE) -DCMAKE_CXX_COMPILER=$(CXX) \
	      -DPGO_GENERATE=ON -DPGO_PROFILE_DIR=$(PGO_DIR)
	cmake --build $(BUILD_DIR) --target $(EXE) -j -- VERBOSE=$(VERBOSE)
	@echo PGO-generate binary built: .$(SEP)$(EXE)
	@echo Run it on workloads to fill $(PGO_DIR)$(SEP)*.profraw

pgo-use: check-compiler
	$(call MKDIR,$(BUILD_DIR))
	$(call MKDIR,$(PGO_DIR))
	cmake -S . -B $(BUILD_DIR) \
	      -DCMAKE_BUILD_TYPE=Release \
	      -DCMAKE_TOOLCHAIN_FILE=clang-toolchain.cmake \
	      -DEXE=$(EXE) -DCMAKE_CXX_COMPILER=$(CXX) \
	      -DPGO_USE=ON -DPGO_PROFILE_DIR=$(PGO_DIR)
	cmake --build $(BUILD_DIR) --target $(EXE) -j -- VERBOSE=$(VERBOSE)
	@echo PGO-use binary built: .$(SEP)$(EXE)
	@echo Profiles used from $(PGO_DIR)$(SEP)merged.profdata

pgo-auto: check-compiler
	$(MAKE) pgo-gen VERBOSE=$(VERBOSE)
	@echo "==> Running workload for profile collection..."
	-./$(EXE) bench
	@echo "==> Merging profiles..."
	llvm-profdata merge -output=$(PGO_DIR)$(SEP)merged.profdata $(PGO_DIR)$(SEP)*.profraw
	$(MAKE) pgo-use VERBOSE=$(VERBOSE)
	@echo "==> Final optimized binary ready: .$(SEP)$(EXE)"

# ===========================
# Clean
# ===========================
clean:
	$(call RMDIR,$(BUILD_DIR))
	$(call RMDIR,$(PGO_DIR))
	$(call RMFILE,$(EXE))
	$(call RMFILE,$(EXE)_test)
	$(call RMFILE,$(EXE)_sanity)

# ===========================
# Help
# ===========================
help:
	@echo "Usage: make [target] [EXE=name] [CXX=clang++] [CONFIG=...] [VERBOSE=1]"
	@echo ""
	@echo "Build targets:"
	@echo "  all (default)     Build with CONFIG (default: Release)"
	@echo "  release           Build release binary (./EXE)"
	@echo "  debug             Build debug binary (./EXE)"
	@echo "  testbuild         Build test binary (./EXE_test)"
	@echo "  sanity            Build sanity binary (./EXE_sanity)"
	@echo ""
	@echo "PGO targets:"
	@echo "  pgo-gen           Build instrumented binary for profile generation"
	@echo "  pgo-use           Build optimized binary using collected profiles"
	@echo "  pgo-auto          Full cycle: pgo-gen → run → merge → pgo-use"
	@echo ""
	@echo "Other:"
	@echo "  clean             Remove build/, pgo-data/, and binaries"
	@echo "  help              Show this help message"
	@echo ""
	@echo "Extra:"
	@echo "  VERBOSE=1         Show full compiler and linker commands"
	@echo ""
	@echo "Preprocessor macros per config:"
	@echo "  Debug    : DEBUG"
	@echo "  Test     : DEBUG, TEST"
	@echo "  Sanity   : TEST"
	@echo "  Release  : NDEBUG (added automatically by CMake)"
