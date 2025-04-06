#include "commons/containers.h"
#include "commons/pch.h"
#include "doctest/doctest.h"
#include "sagittar/core/board.h"
#include "sagittar/core/fen.h"
#include "sagittar/core/move.h"
#include "sagittar/core/movegen.h"
#include "sagittar/core/types.h"

using namespace sagittar;
using namespace sagittar::core::types;

TEST_SUITE("Movegen") {

    TEST_CASE("isSquareAttacked") {
        core::board::Board board;

        std::string fen = "4k3/8/8/4p3/8/8/8/4K3 w - - 0 1";
        core::fen::parseFEN(&board, fen);

        REQUIRE(core::movegen::isSquareAttacked(board, Square::D4, Color::BLACK));
        REQUIRE(core::movegen::isSquareAttacked(board, Square::F4, Color::BLACK));
    }

    TEST_CASE("isInCheck") {
        core::board::Board board;

        std::string fen = "4k3/8/8/8/8/8/3p4/4K3 w - - 0 1";
        core::fen::parseFEN(&board, fen);

        REQUIRE(core::movegen::isInCheck(board));
    }

    TEST_CASE("generatePseudolegalMoves") {
        core::board::Board board;
        board.setStartpos();

        commons::containers::ArrayList<core::move::Move> moves;
        core::movegen::generatePseudolegalMoves(&moves, board, core::movegen::MovegenType::ALL);

        CHECK(moves.size() == 20);
    }
}
