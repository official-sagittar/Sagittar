EXE ?= Sagittar
CXX	?= clang++

PROJECT_ROOT := $(patsubst %/,%,$(dir $(abspath $(lastword $(MAKEFILE_LIST)))))

CMAKE = cmake

BUILD_ROOT = build
BIN_ROOT = bin

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

openbench:
	$(CMAKE) -S . -B $(BUILD_ROOT)/OpenBench \
		-G Ninja \
		-DCMAKE_BUILD_TYPE=Release \
		-DCMAKE_CXX_COMPILER=$(CXX) \
		-DEXE="$(EXE)" \
		-DBIN_DIR="$(PROJECT_ROOT)"
	$(CMAKE) --build $(BUILD_ROOT)/OpenBench --parallel

clean:
	$(CMAKE) -E rm -rf $(BUILD_ROOT) $(BIN_ROOT)
