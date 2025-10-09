#include "board.h"
#include "containers.h"
#include "doctest/doctest.h"
#include "fen.h"
#include "move.h"
#include "movegen.h"
#include "pch.h"
#include "types.h"

using namespace sagittar;

TEST_SUITE("Movegen") {

    TEST_CASE("isSquareAttacked") {
        board::Board board;

        std::string fen = "4k3/8/8/4p3/8/8/8/4K3 w - - 0 1";
        fen::parseFEN(&board, fen);

        REQUIRE(movegen::getSquareAttackers(board, Square::D4, Color::BLACK));
        REQUIRE(movegen::getSquareAttackers(board, Square::F4, Color::BLACK));
    }

    TEST_CASE("isInCheck") {
        board::Board board;

        std::string fen = "4k3/8/8/8/8/8/3p4/4K3 w - - 0 1";
        fen::parseFEN(&board, fen);

        REQUIRE(board.isInCheck());
    }

    TEST_CASE("generatePseudolegalMoves") {
        board::Board board;
        board.setStartpos();

        containers::ArrayList<move::Move> moves;
        movegen::generatePseudolegalMoves(&moves, board, movegen::MovegenType::ALL);

        CHECK(moves.size() == 20);
    }
}
