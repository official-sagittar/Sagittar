#include "core/move.h"
#include "core/position.h"
#include "core/types.h"
#include "doctest/doctest.h"
#include "search/tt.h"

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
        Position pos;
        pos.setFen("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");

        search::TranspositionTable tt(2);

        REQUIRE(tt.getSize() > 0);

        Move m(Square::E2, Square::E4, MoveFlag::MOVE_QUIET_PAWN_DBL_PUSH);

        tt.store(pos.key(), 0, 1, search::TTFlag::EXACT, 10, m);

        search::TTData ttdata;

        bool tthit = tt.probe(&ttdata, pos.key());

        REQUIRE(tthit == true);
        REQUIRE(ttdata.depth == 1);
        REQUIRE(ttdata.flag == search::TTFlag::EXACT);
        REQUIRE(ttdata.score == 10);
        REQUIRE(ttdata.move == m);

        std::string fen = "r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 1";
        pos.setFen(fen);

        m = Move(Square::E2, Square::A6, MoveFlag::MOVE_CAPTURE);

        tt.store(pos.key(), 0, 3, search::TTFlag::LOWERBOUND, 100, m);

        tthit = tt.probe(&ttdata, pos.key());

        REQUIRE(tthit == true);
        REQUIRE(ttdata.depth == 3);
        REQUIRE(ttdata.flag == search::TTFlag::LOWERBOUND);
        REQUIRE(ttdata.score == 100);
        REQUIRE(ttdata.move == m);
    }

    TEST_CASE("Null Move Replacement from the same position") {
        Position pos;

        search::TranspositionTable tt(2);

        REQUIRE(tt.getSize() > 0);

        std::string fen = "r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 1";
        pos.setFen(fen);

        const Move m = Move(Square::E2, Square::A6, MoveFlag::MOVE_CAPTURE);

        tt.store(pos.key(), 0, 3, search::TTFlag::EXACT, 100, m);

        search::TTData ttdata;
        bool           tthit = tt.probe(&ttdata, pos.key());

        REQUIRE(tthit == true);
        REQUIRE(ttdata.depth == 3);
        REQUIRE(ttdata.flag == search::TTFlag::EXACT);
        REQUIRE(ttdata.score == 100);
        REQUIRE(ttdata.move == m);

        const Move nullmove;

        tt.store(pos.key(), 0, 5, search::TTFlag::LOWERBOUND, 50, nullmove);

        tthit = tt.probe(&ttdata, pos.key());

        REQUIRE(tthit == true);
        REQUIRE(ttdata.depth == 5);
        REQUIRE(ttdata.flag == search::TTFlag::LOWERBOUND);
        REQUIRE(ttdata.score == 50);
        REQUIRE(ttdata.move == m);  // move should still be the previous move
    }
}
