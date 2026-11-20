EXE ?= Sagittar
CXX	?= clang++

PROJECT_ROOT := $(patsubst %/,%,$(dir $(abspath $(lastword $(MAKEFILE_LIST)))))

CMAKE = cmake

BUILD_ROOT = build
BIN_ROOT = bin

ifeq ($(OS),Windows_NT)
    EXE_SUFFIX := .exe
else
    EXE_SUFFIX :=
endif

.PHONY: all release debug test tune openbench clean

all: release

release:
	$(CMAKE) -S . -B $(BUILD_ROOT)/Release \
		-G Ninja \
		-DCMAKE_BUILD_TYPE=Release \
		-DCMAKE_CXX_COMPILER=$(CXX) \
		-DEXE="$(EXE)"
	$(CMAKE) --build $(BUILD_ROOT)/Release --parallel

debug:
	$(CMAKE) -S . -B $(BUILD_ROOT)/Debug \
		-G Ninja \
		-DCMAKE_BUILD_TYPE=Debug \
		-DCMAKE_CXX_COMPILER=$(CXX) \
		-DEXE="$(EXE)"
	$(CMAKE) --build $(BUILD_ROOT)/Debug --parallel

test:
	$(CMAKE) -S . -B $(BUILD_ROOT)/Test \
		-G Ninja \
		-DCMAKE_BUILD_TYPE=Test \
		-DCMAKE_CXX_COMPILER=$(CXX) \
		-DEXE="$(EXE)"
	$(CMAKE) --build $(BUILD_ROOT)/Test --parallel

tune:
	$(CMAKE) -S . -B $(BUILD_ROOT)/Tune \
		-G Ninja \
		-DCMAKE_BUILD_TYPE=Tune \
		-DCMAKE_CXX_COMPILER=$(CXX) \
		-DEXE="$(EXE)"
	$(CMAKE) --build $(BUILD_ROOT)/Tune --parallel

openbench: release
	$(CMAKE) -E copy \
		"$(BIN_ROOT)/Release/$(EXE)$(EXE_SUFFIX)" \
		"$(PROJECT_ROOT)"

clean:
	$(CMAKE) -E rm -rf $(BUILD_ROOT) $(BIN_ROOT)
