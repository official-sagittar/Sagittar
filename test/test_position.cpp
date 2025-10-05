#include "core/defs.h"
#include "core/move.h"
#include "core/position.h"
#include "doctest/doctest.h"
#include "pch.h"

using namespace sagittar::core;

TEST_SUITE("Position") {

    TEST_CASE("Position Pinned") {
        Position pos;

        bool is_valid = pos.set_fen("4k3/8/8/b7/8/8/3P4/4K3 w - - 0 1");
        CHECK(is_valid);
        REQUIRE(__builtin_ctzll(pos.pinned) == D2);

        is_valid = pos.set_fen("4k3/8/8/8/8/8/r2PK3/8 w - - 0 1");
        CHECK(is_valid);
        REQUIRE(__builtin_ctzll(pos.pinned) == D2);
    }

    TEST_CASE("Position::is_legal_move") {
        Position pos;

        bool is_valid = pos.set_fen("4k3/4r3/8/8/b7/3B4/4R3/4K3 w - - 0 1");
        CHECK(is_valid);

        Move move = MOVE_CREATE(E2, E3, MOVE_QUIET);
        REQUIRE(pos.is_legal_move(move));

        move = MOVE_CREATE(E2, E7, MOVE_CAPTURE);
        REQUIRE(pos.is_legal_move(move));

        move = MOVE_CREATE(E2, D2, MOVE_QUIET);
        REQUIRE_FALSE(pos.is_legal_move(move));

        move = MOVE_CREATE(E1, D1, MOVE_QUIET);
        REQUIRE_FALSE(pos.is_legal_move(move));

        move = MOVE_CREATE(E1, F1, MOVE_QUIET);
        REQUIRE(pos.is_legal_move(move));

        is_valid = pos.set_fen("4k3/4r3/8/8/3N4/5n2/R7/4K3 w - - 0 1");
        CHECK(is_valid);

        move = MOVE_CREATE(A2, E2, MOVE_QUIET);
        REQUIRE_FALSE(pos.is_legal_move(move));

        move = MOVE_CREATE(D4, F3, MOVE_CAPTURE);
        REQUIRE_FALSE(pos.is_legal_move(move));

        move = MOVE_CREATE(E1, F1, MOVE_QUIET);
        REQUIRE(pos.is_legal_move(move));

        is_valid = pos.set_fen("4k3/4r3/8/8/8/8/R7/4K3 w - - 0 1");
        CHECK(is_valid);

        move = MOVE_CREATE(A2, E2, MOVE_QUIET);
        REQUIRE(pos.is_legal_move(move));

        move = MOVE_CREATE(E1, E2, MOVE_QUIET);
        REQUIRE_FALSE(pos.is_legal_move(move));

        is_valid = pos.set_fen("4k3/2b5/8/3Pp3/8/8/3B3K/8 w - e6 0 1");
        CHECK(is_valid);

        move = MOVE_CREATE(D5, E6, MOVE_CAPTURE_EP);
        REQUIRE_FALSE(pos.is_legal_move(move));

        is_valid = pos.set_fen("4k3/2b5/8/3Pp3/5r2/8/3B3K/8 w - e6 0 1");
        CHECK(is_valid);

        move = MOVE_CREATE(D5, E6, MOVE_CAPTURE_EP);
        REQUIRE(pos.is_legal_move(move));

        is_valid = pos.set_fen("4k3/2b5/8/3Pp3/5r2/8/3B2K1/8 w - e6 0 1");
        CHECK(is_valid);

        move = MOVE_CREATE(D5, E6, MOVE_CAPTURE_EP);
        REQUIRE(pos.is_legal_move(move));

        is_valid = pos.set_fen("4k3/8/8/8/8/8/R3r3/4K3 w - - 0 1");
        CHECK(is_valid);

        move = MOVE_CREATE(A2, E2, MOVE_CAPTURE);
        REQUIRE(pos.is_legal_move(move));

        move = MOVE_CREATE(E1, E2, MOVE_CAPTURE);
        REQUIRE(pos.is_legal_move(move));
    }

    TEST_CASE("Position::is_repeated") {}
}
