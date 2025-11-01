#include "core/move.h"
#include "core/types.h"
#include "doctest/doctest.h"

using namespace sagittar;

TEST_SUITE("Move") {

    TEST_CASE("Move::All") {

        SUBCASE("Default") {
            const Move m;
            CHECK(m.from() == 0);
            CHECK(m.to() == 0);
            CHECK(m.flag() == 0);
        }

        SUBCASE("Comparison") {

            Move e2e4 = Move(Square::E2, Square::E4, MoveFlag::MOVE_QUIET_PAWN_DBL_PUSH);

            CHECK(e2e4.from() == Square::E2);
            CHECK(e2e4.to() == Square::E4);
            CHECK(e2e4.flag() == MoveFlag::MOVE_QUIET_PAWN_DBL_PUSH);

            const Move e2e4_again =
              Move(Square::E2, Square::E4, MoveFlag::MOVE_QUIET_PAWN_DBL_PUSH);

            CHECK(e2e4 == e2e4_again);
        }
    }
}
