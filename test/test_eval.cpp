#include "board.h"
#include "doctest/doctest.h"
#include "eval.h"
#include "fen.h"

using namespace sagittar;

TEST_SUITE("Eval") {
    TEST_CASE("Eval::isInsufficientMaterial") {
        eval::initialize();

        board::Board board;

        // King vs King
        std::string fen = "4k3/8/8/8/8/8/8/4K3 w - - 0 1";
        fen::parseFEN(&board, fen);
        bool is_insufficient = eval::isInsufficientMaterial(board);
        CHECK(is_insufficient == true);

        fen = "4k3/8/8/8/8/8/4P3/4K3 w - - 0 1";
        fen::parseFEN(&board, fen);
        is_insufficient = eval::isInsufficientMaterial(board);
        CHECK(is_insufficient == false);

        // King + Bishop vs King
        fen = "4k3/8/8/8/1B6/8/8/4K3 w - - 0 1";
        fen::parseFEN(&board, fen);
        is_insufficient = eval::isInsufficientMaterial(board);
        CHECK(is_insufficient == true);

        fen = "4k3/8/8/2b5/8/8/8/4K3 w - - 0 1";
        fen::parseFEN(&board, fen);
        is_insufficient = eval::isInsufficientMaterial(board);
        CHECK(is_insufficient == true);

        fen = "4k3/8/8/4p3/1B6/8/8/4K3 w - - 0 1";
        fen::parseFEN(&board, fen);
        is_insufficient = eval::isInsufficientMaterial(board);
        CHECK(is_insufficient == false);

        fen = "4k3/8/8/2b5/8/8/4P3/4K3 w - - 0 1";
        fen::parseFEN(&board, fen);
        is_insufficient = eval::isInsufficientMaterial(board);
        CHECK(is_insufficient == false);

        // King + Knight vs King
        fen = "4k3/8/8/8/2N5/8/8/4K3 w - - 0 1";
        fen::parseFEN(&board, fen);
        is_insufficient = eval::isInsufficientMaterial(board);
        CHECK(is_insufficient == true);

        fen = "4k3/8/8/4n3/8/8/8/4K3 w - - 0 1";
        fen::parseFEN(&board, fen);
        is_insufficient = eval::isInsufficientMaterial(board);
        CHECK(is_insufficient == true);

        fen = "4k3/4p3/8/8/2N5/8/8/4K3 w - - 0 1";
        fen::parseFEN(&board, fen);
        is_insufficient = eval::isInsufficientMaterial(board);
        CHECK(is_insufficient == false);

        fen = "4k3/8/8/8/7n/8/4P3/4K3 w - - 0 1";
        fen::parseFEN(&board, fen);
        is_insufficient = eval::isInsufficientMaterial(board);
        CHECK(is_insufficient == false);

        // King + Bishop vs King + Bishop (both on same color squares)
        fen = "4k3/1b6/8/8/2B5/8/8/4K3 w - - 0 1";
        fen::parseFEN(&board, fen);
        is_insufficient = eval::isInsufficientMaterial(board);
        CHECK(is_insufficient == true);

        fen = "4k3/8/5b2/8/8/8/3B4/4K3 w - - 0 1";
        fen::parseFEN(&board, fen);
        is_insufficient = eval::isInsufficientMaterial(board);
        CHECK(is_insufficient == true);

        fen = "4k3/6b1/8/8/8/8/2B5/4K3 w - - 0 1";
        fen::parseFEN(&board, fen);
        is_insufficient = eval::isInsufficientMaterial(board);
        CHECK(is_insufficient == false);
    }
}
