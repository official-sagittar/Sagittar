#include "board.h"
#include "doctest/doctest.h"
#include "fen.h"
#include "move.h"
#include "tt.h"
#include "types.h"

using namespace sagittar;

TEST_SUITE("TT") {

    TEST_CASE("TranspositionTable::setSize") {
        search::TranspositionTable tt(2);

        std::size_t originalsize = tt.getSize();
        REQUIRE(originalsize > 0);

        tt.setSize(4);
        REQUIRE(tt.getSize() > originalsize);
    }

    TEST_CASE("TranspositionTable::store and TranspositionTable::probe") {
        board::Board board;
        board.setStartpos();

        search::TranspositionTable tt(2);
        REQUIRE(tt.getSize() > 0);

        move::Move m(Square::E2, Square::E4, move::MoveFlag::MOVE_QUIET_PAWN_DBL_PUSH);

        tt.store(board, 1, search::TTFlag::EXACT, 10, m);

        search::TTData ttdata;

        bool tthit = tt.probe(&ttdata, board);

        REQUIRE(tthit == true);
        REQUIRE(ttdata.depth == 1);
        REQUIRE(ttdata.flag == search::TTFlag::EXACT);
        REQUIRE(ttdata.value == 10);
        REQUIRE(ttdata.move == m);

        std::string fen = "r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 1";
        fen::parseFEN(&board, fen);

        m = move::Move(Square::E2, Square::A6, move::MoveFlag::MOVE_CAPTURE);

        tt.store(board, 3, search::TTFlag::LOWERBOUND, 100, m);

        tthit = tt.probe(&ttdata, board);

        REQUIRE(tthit == true);
        REQUIRE(ttdata.depth == 3);
        REQUIRE(ttdata.flag == search::TTFlag::LOWERBOUND);
        REQUIRE(ttdata.value == 100);
        REQUIRE(ttdata.move == m);
    }
}
