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
CFLAGS := -MD -MP -DNDEBUG -I$(SRC_DIR) -m64 -flto -O3 -Wall -Wextra -std=c++20 -march=native
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

sagittar: main.o engine.o uci.o board.o move.o movegen.o perft.o position.o tt.o utils.o search.o
	@echo "==== Building Sagittar ===="
	@$(CXX) $(LDFLAGS) -o $(BIN_DIR)$(PATHSEP)$(EXE) \
		$(OBJ_DIR)$(PATHSEP)main.o \
		$(OBJ_DIR)$(PATHSEP)engine.o \
		$(OBJ_DIR)$(PATHSEP)uci.o \
		$(OBJ_DIR)$(PATHSEP)board.o \
		$(OBJ_DIR)$(PATHSEP)move.o \
		$(OBJ_DIR)$(PATHSEP)movegen.o \
		$(OBJ_DIR)$(PATHSEP)perft.o \
		$(OBJ_DIR)$(PATHSEP)position.o \
		$(OBJ_DIR)$(PATHSEP)tt.o \
		$(OBJ_DIR)$(PATHSEP)utils.o \
		$(OBJ_DIR)$(PATHSEP)search.o

main.o: $(SRC_DIR)$(PATHSEP)main.cpp
	@$(CXX) $(CFLAGS) $(INC_DIR) -c $(SRC_DIR)$(PATHSEP)main.cpp -o $(OBJ_DIR)$(PATHSEP)main.o -MF $(OBJ_DIR)$(PATHSEP)main.d

engine.o: $(SRC_DIR)$(PATHSEP)engine.cpp $(SRC_DIR)$(PATHSEP)engine.h
	@$(CXX) $(CFLAGS) $(INC_DIR) -c $(SRC_DIR)$(PATHSEP)engine.cpp -o $(OBJ_DIR)$(PATHSEP)engine.o -MF $(OBJ_DIR)$(PATHSEP)engine.d

uci.o: $(SRC_DIR)$(PATHSEP)comms$(PATHSEP)uci.cpp $(SRC_DIR)$(PATHSEP)comms$(PATHSEP)uci.h
	@$(CXX) $(CFLAGS) $(INC_DIR) -c $(SRC_DIR)$(PATHSEP)comms$(PATHSEP)uci.cpp -o $(OBJ_DIR)$(PATHSEP)uci.o -MF $(OBJ_DIR)$(PATHSEP)uci.d

board.o: $(SRC_DIR)$(PATHSEP)core$(PATHSEP)board.cpp $(SRC_DIR)$(PATHSEP)core$(PATHSEP)board.h
	@$(CXX) $(CFLAGS) $(INC_DIR) -c $(SRC_DIR)$(PATHSEP)core$(PATHSEP)board.cpp -o $(OBJ_DIR)$(PATHSEP)board.o -MF $(OBJ_DIR)$(PATHSEP)board.d

move.o: $(SRC_DIR)$(PATHSEP)core$(PATHSEP)move.cpp $(SRC_DIR)$(PATHSEP)core$(PATHSEP)move.h
	@$(CXX) $(CFLAGS) $(INC_DIR) -c $(SRC_DIR)$(PATHSEP)core$(PATHSEP)move.cpp -o $(OBJ_DIR)$(PATHSEP)move.o -MF $(OBJ_DIR)$(PATHSEP)move.d

movegen.o: $(SRC_DIR)$(PATHSEP)core$(PATHSEP)movegen.cpp $(SRC_DIR)$(PATHSEP)core$(PATHSEP)movegen.h
	@$(CXX) $(CFLAGS) $(INC_DIR) -c $(SRC_DIR)$(PATHSEP)core$(PATHSEP)movegen.cpp -o $(OBJ_DIR)$(PATHSEP)movegen.o -MF $(OBJ_DIR)$(PATHSEP)movegen.d

perft.o: $(SRC_DIR)$(PATHSEP)core$(PATHSEP)perft.cpp $(SRC_DIR)$(PATHSEP)core$(PATHSEP)perft.h
	@$(CXX) $(CFLAGS) $(INC_DIR) -c $(SRC_DIR)$(PATHSEP)core$(PATHSEP)perft.cpp -o $(OBJ_DIR)$(PATHSEP)perft.o -MF $(OBJ_DIR)$(PATHSEP)perft.d

position.o: $(SRC_DIR)$(PATHSEP)core$(PATHSEP)position.cpp $(SRC_DIR)$(PATHSEP)core$(PATHSEP)position.h
	@$(CXX) $(CFLAGS) $(INC_DIR) -c $(SRC_DIR)$(PATHSEP)core$(PATHSEP)position.cpp -o $(OBJ_DIR)$(PATHSEP)position.o -MF $(OBJ_DIR)$(PATHSEP)position.d

tt.o: $(SRC_DIR)$(PATHSEP)core$(PATHSEP)tt.cpp $(SRC_DIR)$(PATHSEP)core$(PATHSEP)tt.h
	@$(CXX) $(CFLAGS) $(INC_DIR) -c $(SRC_DIR)$(PATHSEP)core$(PATHSEP)tt.cpp -o $(OBJ_DIR)$(PATHSEP)tt.o -MF $(OBJ_DIR)$(PATHSEP)tt.d

utils.o: $(SRC_DIR)$(PATHSEP)core$(PATHSEP)utils.cpp $(SRC_DIR)$(PATHSEP)core$(PATHSEP)utils.h
	@$(CXX) $(CFLAGS) $(INC_DIR) -c $(SRC_DIR)$(PATHSEP)core$(PATHSEP)utils.cpp -o $(OBJ_DIR)$(PATHSEP)utils.o -MF $(OBJ_DIR)$(PATHSEP)utils.d

search.o: $(SRC_DIR)$(PATHSEP)search$(PATHSEP)search.cpp $(SRC_DIR)$(PATHSEP)search$(PATHSEP)search.h
	@$(CXX) $(CFLAGS) $(INC_DIR) -c $(SRC_DIR)$(PATHSEP)search$(PATHSEP)search.cpp -o $(OBJ_DIR)$(PATHSEP)search.o -MF $(OBJ_DIR)$(PATHSEP)search.d

.PHONY: clean dir
