#include "core/move.h"
#include "core/movegen.h"
#include "core/position.h"
#include "doctest/doctest.h"
#include "pch.h"
#include "search/movepicker.h"
#include "search/types.h"

using namespace sagittar::core;
using namespace sagittar::search;

TEST_SUITE("Search::MovePicker") {

    TEST_CASE("MovePicker::next") {
        Position   pos;
        const bool is_valid = pos.set_fen("4k3/8/8/1r1q1n1p/2B1P1P1/2N5/5q2/1R1RK3 w - - 0 1");
        CHECK(is_valid);

        MoveList moves_list = {};
        movegen_generate_pseudolegal_moves<MovegenType::MOVEGEN_ALL>(pos, &moves_list);

        const Move pv_move = MOVE_CREATE(E1, F2, MOVE_CAPTURE);

        size_t size       = 0;
        Score  prev_score = -1;

        History history{};

        MovePicker move_picker(&moves_list, pos, pv_move, &history);
        while (move_picker.has_next())
        {
            const auto [move, move_score] = move_picker.next();

            size++;

            CHECK(move != NULL_MOVE);

            if (prev_score == -1)
            {
                prev_score = move_score;
            }
            else
            {
                CHECK(move_score <= prev_score);
                prev_score = move_score;
            }

            if (move == pv_move)
            {
                CHECK(move_score == 20000);
            }
            else if (MOVE_IS_CAPTURE(move))
            {
                CHECK(move_score >= 15100);
                CHECK(move_score <= 15605);
            }
            else
            {
                CHECK(move_score == 0);
            }
        }

        CHECK(size == moves_list.size);
    }
}
