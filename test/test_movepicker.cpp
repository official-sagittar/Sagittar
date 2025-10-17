#include "board.h"
#include "containers.h"
#include "doctest/doctest.h"
#include "fen.h"
#include "move.h"
#include "movegen.h"
#include "movepicker.h"
#include "pch.h"
#include "search.h"
#include "types.h"

using namespace sagittar;

TEST_SUITE("Movepicker") {

    TEST_CASE("movepicker::sortMoves") {
        search::SearcherData data;

        board::Board board;
        fen::parseFEN(&board, "4k3/8/8/1r1q1n1p/2B1P1P1/2N5/5q2/1R1RK3 w - - 0 1");

        containers::ArrayList<move::Move> moves;
        movegen::generatePseudolegalMoves<movegen::MovegenType::CAPTURES>(&moves, board);

        const move::Move pvmove(Square::E1, Square::F2, move::MoveFlag::MOVE_CAPTURE);

        search::MovePicker move_picker(moves, board, pvmove, data, 0);

        Score prev_score = -1;

        while (move_picker.has_next())
        {
            const move::Move move       = move_picker.next();
            const auto       move_score = move.getScore();

            if (prev_score == -1)
            {
                prev_score = move_score;
            }
            else
            {
                CHECK(move_score <= prev_score);
                prev_score = move_score;
            }

            if (move == pvmove)
            {
                REQUIRE(move.getScore() == 30000);
            }
            else if (move::isCapture(move.getFlag()))
            {
                REQUIRE(move.getScore() >= 10100);
            }
            else
            {
                REQUIRE(move.getScore() == 0);
            }
        }
    }
}
