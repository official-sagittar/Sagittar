#include "board.h"
#include "doctest/doctest.h"
#include "eval.h"
#include "fen.h"

using namespace sagittar;

TEST_SUITE("Eval") {
    TEST_CASE("Eval::eval Insufficient Material") {
        eval::initialize();

        board::Board board;

        // King vs King
        std::string fen = "4k3/8/8/8/8/8/8/4K3 w - - 0 1";
        fen::parseFEN(&board, fen);
        i32 eval = eval::evaluateBoard(board);
        CHECK(eval == 0);

        fen = "4k3/8/8/8/8/8/4P3/4K3 w - - 0 1";
        fen::parseFEN(&board, fen);
        eval = eval::evaluateBoard(board);
        CHECK(eval != 0);

        // King + Bishop vs King
        fen = "4k3/8/8/8/1B6/8/8/4K3 w - - 0 1";
        fen::parseFEN(&board, fen);
        eval = eval::evaluateBoard(board);
        CHECK(eval == 0);

        fen = "4k3/8/8/2b5/8/8/8/4K3 w - - 0 1";
        fen::parseFEN(&board, fen);
        eval = eval::evaluateBoard(board);
        CHECK(eval == 0);

        fen = "4k3/8/8/4p3/1B6/8/8/4K3 w - - 0 1";
        fen::parseFEN(&board, fen);
        eval = eval::evaluateBoard(board);
        CHECK(eval != 0);

        fen = "4k3/8/8/2b5/8/8/4P3/4K3 w - - 0 1";
        fen::parseFEN(&board, fen);
        eval = eval::evaluateBoard(board);
        CHECK(eval != 0);

        // King + Knight vs King
        fen = "4k3/8/8/8/2N5/8/8/4K3 w - - 0 1";
        fen::parseFEN(&board, fen);
        eval = eval::evaluateBoard(board);
        CHECK(eval == 0);

        fen = "4k3/8/8/4n3/8/8/8/4K3 w - - 0 1";
        fen::parseFEN(&board, fen);
        eval = eval::evaluateBoard(board);
        CHECK(eval == 0);

        fen = "4k3/4p3/8/8/2N5/8/8/4K3 w - - 0 1";
        fen::parseFEN(&board, fen);
        eval = eval::evaluateBoard(board);
        CHECK(eval != 0);

        fen = "4k3/8/8/8/7n/8/4P3/4K3 w - - 0 1";
        fen::parseFEN(&board, fen);
        eval = eval::evaluateBoard(board);
        CHECK(eval != 0);

        // King + Bishop vs King + Bishop (both on same color squares)
        fen = "4k3/1b6/8/8/2B5/8/8/4K3 w - - 0 1";
        fen::parseFEN(&board, fen);
        eval = eval::evaluateBoard(board);
        CHECK(eval == 0);

        fen = "4k3/8/5b2/8/8/8/3B4/4K3 w - - 0 1";
        fen::parseFEN(&board, fen);
        eval = eval::evaluateBoard(board);
        CHECK(eval == 0);

        fen = "4k3/4p1b1/8/8/8/8/3B4/4K3 w - - 0 1";
        fen::parseFEN(&board, fen);
        eval = eval::evaluateBoard(board);
        CHECK(eval != 0);
    }
}
