#include "core/move.h"
#include "core/types.h"
#include "doctest/doctest.h"

using namespace sagittar;

TEST_SUITE("Move") {

    TEST_CASE("Move::All") {

        SUBCASE("Default") {
            const Move m;
            CHECK(static_cast<int>(m.from().raw()) == 0);
            CHECK(static_cast<int>(m.to().raw()) == 0);
            CHECK(m.flag() == 0);
        }

        SUBCASE("Comparison") {

            Move e2e4 = Move(Square{Square::Raw::E2}, Square{Square::Raw::E4},
                             MoveFlag::MOVE_QUIET_PAWN_DBL_PUSH);

            CHECK(e2e4.from() == Square::Raw::E2);
            CHECK(e2e4.to() == Square::Raw::E4);
            CHECK(e2e4.flag() == MoveFlag::MOVE_QUIET_PAWN_DBL_PUSH);

            const Move e2e4_again = Move(Square{Square::Raw::E2}, Square{Square::Raw::E4},
                                         MoveFlag::MOVE_QUIET_PAWN_DBL_PUSH);

            CHECK(e2e4 == e2e4_again);
        }
    }
}
