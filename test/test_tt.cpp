#include "board.h"
#include "doctest/doctest.h"
#include "fen.h"
#include "move.h"
#include "tt.h"
#include "types.h"

using namespace sagittar;

TEST_SUITE("TT") {

    TEST_CASE("TranspositionTable::setSize") {
        search::tt::TranspositionTable tt(2);

        std::size_t originalsize = tt.getSize();
        REQUIRE(originalsize > 0);

        tt.setSize(4);
        REQUIRE(tt.getSize() > originalsize);
    }

    TEST_CASE("TranspositionTable::store and TranspositionTable::probe") {
        board::Board board;
        board.setStartpos();

        search::tt::TranspositionTable tt(2);

        REQUIRE(tt.getSize() > 0);

        move::Move m(Square::E2, Square::E4, move::MoveFlag::MOVE_QUIET_PAWN_DBL_PUSH);

        tt.store(board.getHash(), 0, 1, search::tt::TTFlag::EXACT, 10, m);

        search::tt::TTData ttdata;

        bool tthit = tt.probe(&ttdata, board.getHash());

        REQUIRE(tthit == true);
        REQUIRE(ttdata.depth == 1);
        REQUIRE(ttdata.flag == search::tt::TTFlag::EXACT);
        REQUIRE(ttdata.score == 10);
        REQUIRE(ttdata.move == m);

        std::string fen = "r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 1";
        fen::parseFEN(&board, fen);

        m = move::Move(Square::E2, Square::A6, move::MoveFlag::MOVE_CAPTURE);

        tt.store(board.getHash(), 0, 3, search::tt::TTFlag::LOWERBOUND, 100, m);

        tthit = tt.probe(&ttdata, board.getHash());

        REQUIRE(tthit == true);
        REQUIRE(ttdata.depth == 3);
        REQUIRE(ttdata.flag == search::tt::TTFlag::LOWERBOUND);
        REQUIRE(ttdata.score == 100);
        REQUIRE(ttdata.move == m);
    }

    TEST_CASE("Null Move Replacement from the same position") {
        board::Board board;

        search::tt::TranspositionTable tt(2);

        REQUIRE(tt.getSize() > 0);

        std::string fen = "r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 1";
        fen::parseFEN(&board, fen);

        const move::Move m = move::Move(Square::E2, Square::A6, move::MoveFlag::MOVE_CAPTURE);

        tt.store(board.getHash(), 0, 3, search::tt::TTFlag::EXACT, 100, m);

        search::tt::TTData ttdata;
        bool               tthit = tt.probe(&ttdata, board.getHash());

        REQUIRE(tthit == true);
        REQUIRE(ttdata.depth == 3);
        REQUIRE(ttdata.flag == search::tt::TTFlag::EXACT);
        REQUIRE(ttdata.score == 100);
        REQUIRE(ttdata.move == m);

        const move::Move nullmove;

        tt.store(board.getHash(), 0, 5, search::tt::TTFlag::LOWERBOUND, 50, nullmove);

        tthit = tt.probe(&ttdata, board.getHash());

        REQUIRE(tthit == true);
        REQUIRE(ttdata.depth == 5);
        REQUIRE(ttdata.flag == search::tt::TTFlag::LOWERBOUND);
        REQUIRE(ttdata.score == 50);
        REQUIRE(ttdata.move == m);  // move should still be the previous move
    }
}
