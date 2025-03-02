#include "board.h"
#include "containers.h"
#include "doctest/doctest.h"
#include "fen.h"
#include "move.h"
#include "movegen.h"
#include "movepicker.h"
#include "pch.h"
#include "search.h"
#include "tt.h"
#include "types.h"

using namespace sagittar;

TEST_SUITE("Movepicker") {

    TEST_CASE("movepicker::sortMoves") {
        search::tt::TranspositionTable tt(2);
        search::SearcherData           data;

        board::Board board;
        fen::parseFEN(&board, "4k3/8/8/1r1q1n1p/2B1P1P1/2N5/5q2/1R1RK3 w - - 0 1");

        containers::ArrayList<move::Move> moves;
        movegen::generatePseudolegalMoves(&moves, board, movegen::MovegenType::ALL);

        const move::Move pvmove(Square::E1, Square::F2, move::MoveFlag::MOVE_CAPTURE);

        search::scoreMoves(&moves, board, pvmove, tt, data, 0);
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
