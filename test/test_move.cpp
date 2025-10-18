#include "doctest/doctest.h"
#include "move.h"
#include "types.h"

using namespace sagittar;

TEST_SUITE("Move") {

    TEST_CASE("Move::All") {

        SUBCASE("Default") {
            const move::Move m;
            CHECK(m.getFrom() == Square::NO_SQ);
            CHECK(m.getTo() == Square::NO_SQ);
            CHECK(m.getFlag() == move::MoveFlag::MOVE_QUIET);
        }

        SUBCASE("Comparison") {

            move::Move e2e4 =
              move::Move(Square::E2, Square::E4, move::MoveFlag::MOVE_QUIET_PAWN_DBL_PUSH);

            CHECK(e2e4.getFrom() == Square::E2);
            CHECK(e2e4.getTo() == Square::E4);
            CHECK(e2e4.getFlag() == move::MoveFlag::MOVE_QUIET_PAWN_DBL_PUSH);

            const move::Move e2e4_again =
              move::Move(Square::E2, Square::E4, move::MoveFlag::MOVE_QUIET_PAWN_DBL_PUSH);

            CHECK(e2e4 == e2e4_again);
        }
    }
}
