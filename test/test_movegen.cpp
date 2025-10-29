#include "containers.h"
#include "doctest/doctest.h"
#include "move.h"
#include "movegen.h"
#include "pch.h"
#include "position.h"
#include "types.h"

using namespace sagittar;

TEST_SUITE("Movegen") {

    TEST_CASE("isSquareAttacked") {
        core::Position pos;

        std::string fen = "4k3/8/8/4p3/8/8/8/4K3 w - - 0 1";
        pos.setFen(fen);

        REQUIRE(movegen::getSquareAttackers(pos, Square::D4, Color::BLACK));
        REQUIRE(movegen::getSquareAttackers(pos, Square::F4, Color::BLACK));
    }

    TEST_CASE("isInCheck") {
        core::Position pos;

        std::string fen = "4k3/8/8/8/8/8/3p4/4K3 w - - 0 1";
        pos.setFen(fen);

        REQUIRE(pos.isInCheck());
    }

    TEST_CASE("generatePseudolegalMoves") {
        core::Position pos;
        pos.setFen("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");

        containers::ArrayList<move::Move> moves;
        movegen::generatePseudolegalMoves<movegen::MovegenType::ALL>(&moves, pos);

        CHECK(moves.size() == 20);
    }
}
