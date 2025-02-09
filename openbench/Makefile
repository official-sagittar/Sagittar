EXE ?= sagittar
CXX ?= clang++
CFLAGS := -MD -MP -DNDEBUG -m64 -Werror -flto -O3 -std=c++20 -march=native
LDFLAGS := -m64 -static -flto

BIN_DIR := .
OBJ_DIR := ../obj
SRC_DIR := ../src
INC_DIR := -I$(SRC_DIR)

all: clean dir sagittar

clean:
	@rm -rf $(OBJ_DIR)
	@rm -f $(EXE)

dir:
	@mkdir -p $(OBJ_DIR)

sagittar: main.o board.o engine.o eval.o fen.o move.o movegen.o movepicker.o perft.o search.o tt.o uci.o utils.o
	@echo "==== Building sagittar ===="
	@$(CXX) $(LDFLAGS) -o $(BIN_DIR)/$(EXE) $(OBJ_DIR)/main.o \
		$(OBJ_DIR)/board.o \
		$(OBJ_DIR)/engine.o \
		$(OBJ_DIR)/eval.o \
		$(OBJ_DIR)/fen.o \
		$(OBJ_DIR)/move.o \
		$(OBJ_DIR)/movegen.o \
		$(OBJ_DIR)/movepicker.o \
		$(OBJ_DIR)/perft.o \
		$(OBJ_DIR)/search.o \
		$(OBJ_DIR)/tt.o \
		$(OBJ_DIR)/uci.o \
		$(OBJ_DIR)/utils.o

main.o: $(SRC_DIR)/main.cpp
	@$(CXX) $(CFLAGS) $(INC_DIR) -c $(SRC_DIR)/main.cpp -o $(OBJ_DIR)/main.o -MF $(OBJ_DIR)/main.d

board.o: $(SRC_DIR)/board.cpp $(SRC_DIR)/board.h
	@$(CXX) $(CFLAGS) $(INC_DIR) -c $(SRC_DIR)/board.cpp -o $(OBJ_DIR)/board.o -MF $(OBJ_DIR)/board.d

engine.o: $(SRC_DIR)/engine.cpp $(SRC_DIR)/engine.h
	@$(CXX) $(CFLAGS) $(INC_DIR) -c $(SRC_DIR)/engine.cpp -o $(OBJ_DIR)/engine.o -MF $(OBJ_DIR)/engine.d

eval.o: $(SRC_DIR)/eval.cpp $(SRC_DIR)/eval.h
	@$(CXX) $(CFLAGS) $(INC_DIR) -c $(SRC_DIR)/eval.cpp -o $(OBJ_DIR)/eval.o -MF $(OBJ_DIR)/eval.d

fen.o: $(SRC_DIR)/fen.cpp $(SRC_DIR)/fen.h
	@$(CXX) $(CFLAGS) $(INC_DIR) -c $(SRC_DIR)/fen.cpp -o $(OBJ_DIR)/fen.o -MF $(OBJ_DIR)/fen.d

move.o: $(SRC_DIR)/move.cpp $(SRC_DIR)/move.h
	@$(CXX) $(CFLAGS) $(INC_DIR) -c $(SRC_DIR)/move.cpp -o $(OBJ_DIR)/move.o -MF $(OBJ_DIR)/move.d

movegen.o: $(SRC_DIR)/movegen.cpp $(SRC_DIR)/movegen.h
	@$(CXX) $(CFLAGS) $(INC_DIR) -c $(SRC_DIR)/movegen.cpp -o $(OBJ_DIR)/movegen.o -MF $(OBJ_DIR)/movegen.d

movepicker.o: $(SRC_DIR)/movepicker.cpp $(SRC_DIR)/movepicker.h
	@$(CXX) $(CFLAGS) $(INC_DIR) -c $(SRC_DIR)/movepicker.cpp -o $(OBJ_DIR)/movepicker.o -MF $(OBJ_DIR)/movepicker.d

perft.o: $(SRC_DIR)/perft.cpp $(SRC_DIR)/perft.h
	@$(CXX) $(CFLAGS) $(INC_DIR) -c $(SRC_DIR)/perft.cpp -o $(OBJ_DIR)/perft.o -MF $(OBJ_DIR)/perft.d

search.o: $(SRC_DIR)/search.cpp $(SRC_DIR)/search.h
	@$(CXX) $(CFLAGS) $(INC_DIR) -c $(SRC_DIR)/search.cpp -o $(OBJ_DIR)/search.o -MF $(OBJ_DIR)/search.d

tt.o: $(SRC_DIR)/tt.cpp $(SRC_DIR)/tt.h
	@$(CXX) $(CFLAGS) $(INC_DIR) -c $(SRC_DIR)/tt.cpp -o $(OBJ_DIR)/tt.o -MF $(OBJ_DIR)/tt.d

uci.o: $(SRC_DIR)/uci.cpp $(SRC_DIR)/uci.h
	@$(CXX) $(CFLAGS) $(INC_DIR) -c $(SRC_DIR)/uci.cpp -o $(OBJ_DIR)/uci.o -MF $(OBJ_DIR)/uci.d

utils.o: $(SRC_DIR)/utils.cpp $(SRC_DIR)/utils.h
	@$(CXX) $(CFLAGS) $(INC_DIR) -c $(SRC_DIR)/utils.cpp -o $(OBJ_DIR)/utils.o -MF $(OBJ_DIR)/utils.d

.PHONY: clean dir
