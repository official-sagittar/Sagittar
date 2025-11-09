#include "commons/containers.h"
#include "commons/pch.h"
#include "core/move.h"
#include "core/movegen.h"
#include "core/position.h"
#include "core/types.h"
#include "doctest/doctest.h"

using namespace sagittar;

TEST_SUITE("Movegen") {

    TEST_CASE("squareAttackers") {
        Position pos;

        std::string fen = "4k3/8/8/4p3/8/8/8/4K3 w - - 0 1";
        pos.setFen(fen);

        REQUIRE(squareAttackers(pos, Square::D4, Color::BLACK));
        REQUIRE(squareAttackers(pos, Square::F4, Color::BLACK));
    }

    TEST_CASE("isInCheck") {
        Position pos;

        std::string fen = "4k3/8/8/8/8/8/3p4/4K3 w - - 0 1";
        pos.setFen(fen);

        REQUIRE(pos.isInCheck());
    }

    TEST_CASE("pseudolegalMoves") {
        Position pos;
        pos.setFen("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");

        containers::ArrayList<Move> moves;
        pseudolegalMoves<MovegenType::ALL>(&moves, pos);

        CHECK(moves.size() == 20);
    }
}
