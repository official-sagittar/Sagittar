#include "doctest/doctest.h"
#include "sagittar/core/move.h"
#include "sagittar/core/types.h"

using namespace sagittar;
using namespace sagittar::core::types;

TEST_SUITE("Move") {

    TEST_CASE("Move::All") {

        SUBCASE("Default") {
            const core::move::Move m;
            CHECK(m.getFrom() == Square::NO_SQ);
            CHECK(m.getTo() == Square::NO_SQ);
            CHECK(m.getFlag() == core::move::MoveFlag::MOVE_QUIET);
            CHECK(m.getScore() == 0);
        }

        SUBCASE("Comparison") {

            core::move::Move e2e4 = core::move::Move(
              Square::E2, Square::E4, core::move::MoveFlag::MOVE_QUIET_PAWN_DBL_PUSH);

            CHECK(e2e4.getFrom() == Square::E2);
            CHECK(e2e4.getTo() == Square::E4);
            CHECK(e2e4.getFlag() == core::move::MoveFlag::MOVE_QUIET_PAWN_DBL_PUSH);
            CHECK(e2e4.getScore() == 0);

            const core::move::Move e2e4_again = core::move::Move(
              Square::E2, Square::E4, core::move::MoveFlag::MOVE_QUIET_PAWN_DBL_PUSH);

            CHECK(e2e4 == e2e4_again);
        }
    }
}
