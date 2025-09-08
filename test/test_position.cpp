#include "core/position.h"
#include "doctest/doctest.h"
#include "pch.h"

using namespace sagittar::core;

TEST_SUITE("Position") {

    TEST_CASE("Position::is_repeated") {
        Position        pos;
        PositionHistory history;

        const bool is_valid =
          pos.set_fen("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");
        CHECK(is_valid);

        CHECK(pos.do_move("g1f3", &history));
        REQUIRE_FALSE(pos.is_repeated(&history));

        CHECK(pos.do_move("g8f6", &history));
        REQUIRE_FALSE(pos.is_repeated(&history));

        CHECK(pos.do_move("f3g1", &history));
        REQUIRE_FALSE(pos.is_repeated(&history));

        CHECK(pos.do_move("f6g8", &history));
        REQUIRE(pos.is_repeated(&history));

        CHECK(pos.do_move("g1f3", &history));
        REQUIRE(pos.is_repeated(&history));

        CHECK(pos.do_move("g8f6", &history));
        REQUIRE(pos.is_repeated(&history));

        CHECK(pos.do_move("f3g1", &history));
        REQUIRE(pos.is_repeated(&history));

        CHECK(pos.do_move("f6g8", &history));
        REQUIRE(pos.is_repeated(&history));
    }
}
