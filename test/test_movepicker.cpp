#include "commons/containers.h"
#include "commons/pch.h"
#include "doctest/doctest.h"
#include "sagittar/core/board.h"
#include "sagittar/core/fen.h"
#include "sagittar/core/move.h"
#include "sagittar/core/movegen.h"
#include "sagittar/core/types.h"
#include "sagittar/search/data.h"
#include "sagittar/search/movepicker.h"

using namespace sagittar;
using namespace sagittar::core::types;

TEST_SUITE("Movepicker") {

    TEST_CASE("movepicker::sortMoves") {
        search::data::SearcherData data;

        core::board::Board board;
        core::fen::parseFEN(&board, "4k3/8/8/1r1q1n1p/2B1P1P1/2N5/5q2/1R1RK3 w - - 0 1");

        commons::containers::ArrayList<core::move::Move> moves;
        core::movegen::generatePseudolegalMoves(&moves, board, core::movegen::MovegenType::ALL);

        const core::move::Move pvmove(Square::E1, Square::F2, core::move::MoveFlag::MOVE_CAPTURE);
        const core::move::Move ttmove;

        search::movepicker::scoreMoves(&moves, board, pvmove, ttmove, data, 0);
        for (u8 i = 1; i < moves.size(); i++)
        {
            if (moves.at(i) == pvmove)
            {
                REQUIRE(moves.at(i).getScore() == 40000);
            }
            else if (core::move::isCapture(moves.at(i).getFlag()))
            {
                REQUIRE(moves.at(i).getScore() >= 10100);
            }
            else
            {
                REQUIRE(moves.at(i).getScore() == 0);
            }
        }

        search::movepicker::sortMoves(&moves, 0);

        REQUIRE(moves.at(0).getScore() == 40000);

        for (u8 i = 1; i < moves.size(); i++)
        {
            search::movepicker::sortMoves(&moves, i);
            REQUIRE(moves.at(i).getScore() <= moves.at(i - 1).getScore());
        }
    }
}
