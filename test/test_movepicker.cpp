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
        movegen::generatePseudolegalMoves<movegen::MovegenType::ALL>(&moves, board);

        const move::Move pvmove(Square::E1, Square::F2, move::MoveFlag::MOVE_CAPTURE);
        const move::Move ttmove;

        search::scoreMoves(&moves, board, pvmove, ttmove, data, 0);
        for (u8 i = 1; i < moves.size(); i++)
        {
            if (moves.at(i) == pvmove)
            {
                REQUIRE(moves.at(i).getScore() == 40000);
            }
            else if (move::isCapture(moves.at(i).getFlag()))
            {
                REQUIRE(moves.at(i).getScore() >= 10100);
            }
            else
            {
                REQUIRE(moves.at(i).getScore() == 0);
            }
        }

        search::sortMoves(&moves, 0);

        REQUIRE(moves.at(0).getScore() == 40000);

        for (u8 i = 1; i < moves.size(); i++)
        {
            search::sortMoves(&moves, i);
            REQUIRE(moves.at(i).getScore() <= moves.at(i - 1).getScore());
        }
    }
}
