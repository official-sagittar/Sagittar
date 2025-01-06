#include "board.h"
#include "containers.h"
#include "doctest/doctest.h"
#include "fen.h"
#include "move.h"
#include "movegen.h"
#include "movepicker.h"
#include "pch.h"
#include "types.h"

using namespace sagittar;

TEST_SUITE("Movepicker") {

    TEST_CASE("movepicker::sortMoves") {
        board::Board board;
        fen::parseFEN(&board, "4k3/8/8/1r1q1n1p/2B1P1P1/2N5/5q2/1R1RK3 w - - 0 1");

        containers::ArrayList<move::Move> moves;
        movegen::generatePseudolegalMoves(&moves, board, movegen::MovegenType::ALL);

        search::scoreMoves(&moves, board);
        for (u8 i = 1; i < moves.size(); i++)
        {
            if (move::isCapture(moves.at(i).getFlag()))
            {
                REQUIRE(moves.at(i).getScore() >= 10100);
            }
            else
            {
                REQUIRE(moves.at(i).getScore() == 0);
            }
        }

        search::sortMoves(&moves, 0);

        for (u8 i = 1; i < moves.size(); i++)
        {
            search::sortMoves(&moves, i);
            REQUIRE(moves.at(i).getScore() <= moves.at(i - 1).getScore());
        }
    }
}
