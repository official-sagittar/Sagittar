EXE        ?= Sagittar
BIN_DIR    ?= .
ROOT_DIR   := $(realpath $(dir $(lastword $(MAKEFILE_LIST)))/..)
BUILD_ROOT := $(ROOT_DIR)/build
override CXX := clang++

ifeq ($(OS),Windows_NT)
	IS_WINDOWS := 1
	DEL_CMD := if exist "$(subst /,\,$(BIN_DIR))\\$(EXE).exe" del /Q "$(subst /,\,$(BIN_DIR))\\$(EXE).exe" & if exist "$(subst /,\,$(BIN_DIR))\\$(EXE)" del /Q "$(subst /,\,$(BIN_DIR))\\$(EXE)"
	RMDIR_CMD := if exist "$(subst /,\,$(BUILD_ROOT))" rmdir /S /Q "$(subst /,\,$(BUILD_ROOT))"
	MKDIR_CMD := if not exist "$(subst /,\,$(BUILD_ROOT))\\$(1)" mkdir "$(subst /,\,$(BUILD_ROOT))\\$(1)"
	SILENT_NULL := >nul 2>&1
else
	IS_WINDOWS := 0
	DEL_CMD := rm -f "$(BIN_DIR)/$(EXE)" "$(BIN_DIR)/$(EXE).exe"
	RMDIR_CMD := rm -rf "$(BUILD_ROOT)"
	MKDIR_CMD := mkdir -p "$(BUILD_ROOT)/$(1)"
	SILENT_NULL := > /dev/null 2>&1
endif

define run_cmake
	@echo "=== Configuring $(1) build ==="
	@$(call MKDIR_CMD,$(1)) $(SILENT_NULL) || true
	cmake -S "$(ROOT_DIR)" -B "$(BUILD_ROOT)/$(1)" \
		-G "Ninja" \
		-DCMAKE_BUILD_TYPE=$(1) \
		-DENABLE_SAN=$(2) \
		-DEXE=$(EXE) \
		-DBIN_DIR=$(BIN_DIR) \
		-DCMAKE_CXX_COMPILER=$(CXX)
	@echo "=== Building $(EXE) ($(1)) ==="
	cmake --build "$(BUILD_ROOT)/$(1)" --target $(EXE)
	@echo "=== Build complete: $(BIN_DIR)/$(EXE) ==="
endef

.PHONY: all release clean

all: clean release

release:
	$(call run_cmake,Tune,OFF)

clean:
	@echo "=== Cleaning build artifacts ==="
ifeq ($(IS_WINDOWS),1)
	@$(RMDIR_CMD)
	@$(DEL_CMD)
else
	@$(RMDIR_CMD)
	@$(DEL_CMD)
endif
