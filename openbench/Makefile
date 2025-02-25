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

EXE ?= Sagittar$(EXE_EXT)
CXX ?= clang++
CFLAGS := -MD -MP -DNDEBUG -m64 -Werror -flto -O3 -std=c++20 -march=native
LDFLAGS := -m64 -static -flto

BIN_DIR := .
OBJ_DIR := ..$(PATHSEP)obj
SRC_DIR := ..$(PATHSEP)src
INC_DIR := -I$(SRC_DIR)

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

sagittar: main.o board.o engine.o eval.o fen.o move.o movegen.o movepicker.o perft.o search.o tt.o uci.o utils.o
	@echo "==== Building Sagittar ===="
	@$(CXX) $(LDFLAGS) -o $(BIN_DIR)$(PATHSEP)$(EXE) \
		$(OBJ_DIR)$(PATHSEP)main.o \
		$(OBJ_DIR)$(PATHSEP)board.o \
		$(OBJ_DIR)$(PATHSEP)engine.o \
		$(OBJ_DIR)$(PATHSEP)eval.o \
		$(OBJ_DIR)$(PATHSEP)fen.o \
		$(OBJ_DIR)$(PATHSEP)move.o \
		$(OBJ_DIR)$(PATHSEP)movegen.o \
		$(OBJ_DIR)$(PATHSEP)movepicker.o \
		$(OBJ_DIR)$(PATHSEP)perft.o \
		$(OBJ_DIR)$(PATHSEP)search.o \
		$(OBJ_DIR)$(PATHSEP)tt.o \
		$(OBJ_DIR)$(PATHSEP)uci.o \
		$(OBJ_DIR)$(PATHSEP)utils.o

main.o: $(SRC_DIR)$(PATHSEP)main.cpp
	@$(CXX) $(CFLAGS) $(INC_DIR) -c $(SRC_DIR)$(PATHSEP)main.cpp -o $(OBJ_DIR)$(PATHSEP)main.o -MF $(OBJ_DIR)$(PATHSEP)main.d

board.o: $(SRC_DIR)$(PATHSEP)board.cpp $(SRC_DIR)$(PATHSEP)board.h
	@$(CXX) $(CFLAGS) $(INC_DIR) -c $(SRC_DIR)$(PATHSEP)board.cpp -o $(OBJ_DIR)$(PATHSEP)board.o -MF $(OBJ_DIR)$(PATHSEP)board.d

engine.o: $(SRC_DIR)$(PATHSEP)engine.cpp $(SRC_DIR)$(PATHSEP)engine.h
	@$(CXX) $(CFLAGS) $(INC_DIR) -c $(SRC_DIR)$(PATHSEP)engine.cpp -o $(OBJ_DIR)$(PATHSEP)engine.o -MF $(OBJ_DIR)$(PATHSEP)engine.d

eval.o: $(SRC_DIR)$(PATHSEP)eval.cpp $(SRC_DIR)$(PATHSEP)eval.h
	@$(CXX) $(CFLAGS) $(INC_DIR) -c $(SRC_DIR)$(PATHSEP)eval.cpp -o $(OBJ_DIR)$(PATHSEP)eval.o -MF $(OBJ_DIR)$(PATHSEP)eval.d

fen.o: $(SRC_DIR)$(PATHSEP)fen.cpp $(SRC_DIR)$(PATHSEP)fen.h
	@$(CXX) $(CFLAGS) $(INC_DIR) -c $(SRC_DIR)$(PATHSEP)fen.cpp -o $(OBJ_DIR)$(PATHSEP)fen.o -MF $(OBJ_DIR)$(PATHSEP)fen.d

move.o: $(SRC_DIR)$(PATHSEP)move.cpp $(SRC_DIR)$(PATHSEP)move.h
	@$(CXX) $(CFLAGS) $(INC_DIR) -c $(SRC_DIR)$(PATHSEP)move.cpp -o $(OBJ_DIR)$(PATHSEP)move.o -MF $(OBJ_DIR)$(PATHSEP)move.d

movegen.o: $(SRC_DIR)$(PATHSEP)movegen.cpp $(SRC_DIR)$(PATHSEP)movegen.h
	@$(CXX) $(CFLAGS) $(INC_DIR) -c $(SRC_DIR)$(PATHSEP)movegen.cpp -o $(OBJ_DIR)$(PATHSEP)movegen.o -MF $(OBJ_DIR)$(PATHSEP)movegen.d

movepicker.o: $(SRC_DIR)$(PATHSEP)movepicker.cpp $(SRC_DIR)$(PATHSEP)movepicker.h
	@$(CXX) $(CFLAGS) $(INC_DIR) -c $(SRC_DIR)$(PATHSEP)movepicker.cpp -o $(OBJ_DIR)$(PATHSEP)movepicker.o -MF $(OBJ_DIR)$(PATHSEP)movepicker.d

perft.o: $(SRC_DIR)$(PATHSEP)perft.cpp $(SRC_DIR)$(PATHSEP)perft.h
	@$(CXX) $(CFLAGS) $(INC_DIR) -c $(SRC_DIR)$(PATHSEP)perft.cpp -o $(OBJ_DIR)$(PATHSEP)perft.o -MF $(OBJ_DIR)$(PATHSEP)perft.d

search.o: $(SRC_DIR)$(PATHSEP)search.cpp $(SRC_DIR)$(PATHSEP)search.h
	@$(CXX) $(CFLAGS) $(INC_DIR) -c $(SRC_DIR)$(PATHSEP)search.cpp -o $(OBJ_DIR)$(PATHSEP)search.o -MF $(OBJ_DIR)$(PATHSEP)search.d

tt.o: $(SRC_DIR)$(PATHSEP)tt.cpp $(SRC_DIR)$(PATHSEP)tt.h
	@$(CXX) $(CFLAGS) $(INC_DIR) -c $(SRC_DIR)$(PATHSEP)tt.cpp -o $(OBJ_DIR)$(PATHSEP)tt.o -MF $(OBJ_DIR)$(PATHSEP)tt.d

uci.o: $(SRC_DIR)$(PATHSEP)uci.cpp $(SRC_DIR)$(PATHSEP)uci.h
	@$(CXX) $(CFLAGS) $(INC_DIR) -c $(SRC_DIR)$(PATHSEP)uci.cpp -o $(OBJ_DIR)$(PATHSEP)uci.o -MF $(OBJ_DIR)$(PATHSEP)uci.d

utils.o: $(SRC_DIR)$(PATHSEP)utils.cpp $(SRC_DIR)$(PATHSEP)utils.h
	@$(CXX) $(CFLAGS) $(INC_DIR) -c $(SRC_DIR)$(PATHSEP)utils.cpp -o $(OBJ_DIR)$(PATHSEP)utils.o -MF $(OBJ_DIR)$(PATHSEP)utils.d

.PHONY: clean dir
