#include "commons/pch.h"
#include "core/move.h"
#include "core/position.h"
#include "core/types.h"
#include "doctest/doctest.h"

using namespace sagittar;

TEST_SUITE("Position") {
    TEST_CASE("Position::pieceCount") {
        core::Position pos;
        pos.setFen("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");

        CHECK(pos.pieceCount(Piece::WHITE_KING) == 1);
        CHECK(pos.pieceCount(Piece::WHITE_QUEEN) == 1);
        CHECK(pos.pieceCount(Piece::WHITE_ROOK) == 2);
        CHECK(pos.pieceCount(Piece::WHITE_BISHOP) == 2);
        CHECK(pos.pieceCount(Piece::WHITE_KNIGHT) == 2);
        CHECK(pos.pieceCount(Piece::WHITE_PAWN) == 8);
        CHECK(pos.pieceCount(Piece::WHITE_ROOK) == 2);

        CHECK(pos.pieceCount(Piece::BLACK_KING) == 1);
        CHECK(pos.pieceCount(Piece::BLACK_QUEEN) == 1);
        CHECK(pos.pieceCount(Piece::BLACK_ROOK) == 2);
        CHECK(pos.pieceCount(Piece::BLACK_BISHOP) == 2);
        CHECK(pos.pieceCount(Piece::BLACK_KNIGHT) == 2);
        CHECK(pos.pieceCount(Piece::BLACK_PAWN) == 8);
        CHECK(pos.pieceCount(Piece::BLACK_ROOK) == 2);
    }

    TEST_CASE("Position::doMove::string") {
        core::Position pos;

        // MOVE_QUIET_PAWN_DBL_PUSH
        pos.setFen("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");
        bool is_valid = (pos.doMove("e2e4") == core::DoMoveResult::LEGAL);
        CHECK(is_valid);
        CHECK(pos.pieceOn(Square::E2) == Piece::NO_PIECE);
        CHECK(pos.pieceOn(Square::E4) == Piece::WHITE_PAWN);

        // MOVE_CASTLE_KING_SIDE WHITE
        std::string fen = "r1bqkb1r/pppp1ppp/2n2n2/4p3/2B1P3/5N2/PPPP1PPP/RNBQK2R w KQkq - 4 4";
        pos.setFen(fen);
        is_valid = (pos.doMove("e1g1") == core::DoMoveResult::LEGAL);
        CHECK(is_valid);
        CHECK(pos.pieceOn(Square::E1) == Piece::NO_PIECE);
        CHECK(pos.pieceOn(Square::F1) == Piece::WHITE_ROOK);
        CHECK(pos.pieceOn(Square::G1) == Piece::WHITE_KING);
        CHECK(pos.pieceOn(Square::H1) == Piece::NO_PIECE);

        // MOVE_CASTLE_KING_SIDE BLACK
        fen = "r1bqk2r/pppp1ppp/2n2n2/2b1p3/2B1P3/3P1N2/PPP2PPP/RNBQ1RK1 b kq - 0 5";
        pos.setFen(fen);
        is_valid = (pos.doMove("e8g8") == core::DoMoveResult::LEGAL);
        CHECK(is_valid);
        CHECK(pos.pieceOn(Square::E8) == Piece::NO_PIECE);
        CHECK(pos.pieceOn(Square::F8) == Piece::BLACK_ROOK);
        CHECK(pos.pieceOn(Square::G8) == Piece::BLACK_KING);
        CHECK(pos.pieceOn(Square::H8) == Piece::NO_PIECE);

        // MOVE_CASTLE_QUEEN_SIDE WHITE
        fen = "r3kb1r/pp1npppp/2p2n2/q4b2/3P1B2/2N2N2/PPPQ1PPP/R3KB1R w KQkq - 6 8";
        pos.setFen(fen);
        is_valid = (pos.doMove("e1c1") == core::DoMoveResult::LEGAL);
        CHECK(is_valid);
        CHECK(pos.pieceOn(Square::E1) == Piece::NO_PIECE);
        CHECK(pos.pieceOn(Square::D1) == Piece::WHITE_ROOK);
        CHECK(pos.pieceOn(Square::C1) == Piece::WHITE_KING);
        CHECK(pos.pieceOn(Square::B1) == Piece::NO_PIECE);
        CHECK(pos.pieceOn(Square::A1) == Piece::NO_PIECE);

        // MOVE_CASTLE_QUEEN_SIDE BLACK
        fen = "r3kb1r/pp1npppp/2p2n2/q4b2/3P1B2/2N2N2/PPPQ1PPP/2KR1B1R b kq - 7 8";
        pos.setFen(fen);
        is_valid = (pos.doMove("e8c8") == core::DoMoveResult::LEGAL);
        CHECK(is_valid);
        CHECK(pos.pieceOn(Square::E8) == Piece::NO_PIECE);
        CHECK(pos.pieceOn(Square::D8) == Piece::BLACK_ROOK);
        CHECK(pos.pieceOn(Square::C8) == Piece::BLACK_KING);
        CHECK(pos.pieceOn(Square::B8) == Piece::NO_PIECE);
        CHECK(pos.pieceOn(Square::A8) == Piece::NO_PIECE);

        // MOVE_CAPTURE_EP

        // WHITE
        fen = "4k3/8/8/3Pp3/8/8/8/4K3 w - e6 0 1";
        pos.setFen(fen);
        is_valid = (pos.doMove("d5e6") == core::DoMoveResult::LEGAL);
        CHECK(is_valid);
        CHECK(pos.pieceOn(Square::D5) == Piece::NO_PIECE);
        CHECK(pos.pieceOn(Square::E5) == Piece::NO_PIECE);
        CHECK(pos.pieceOn(Square::E6) == Piece::WHITE_PAWN);

        // BLACK
        fen = "4k3/8/8/8/3pP3/8/8/4K3 b - e3 0 1";
        pos.setFen(fen);
        is_valid = (pos.doMove("d4e3") == core::DoMoveResult::LEGAL);
        CHECK(is_valid);
        CHECK(pos.pieceOn(Square::D4) == Piece::NO_PIECE);
        CHECK(pos.pieceOn(Square::E4) == Piece::NO_PIECE);
        CHECK(pos.pieceOn(Square::E3) == Piece::BLACK_PAWN);

        // MOVE_PROMOTION
        fen = "1q2k3/P7/8/8/8/8/8/4K3 w - - 0 1";
        pos.setFen(fen);

        // KNIGHT
        core::Position pos_copy = pos;
        is_valid                = (pos_copy.doMove("a7a8n") == core::DoMoveResult::LEGAL);
        CHECK(is_valid);
        CHECK(pos_copy.pieceOn(Square::A7) == Piece::NO_PIECE);
        CHECK(pos_copy.pieceOn(Square::A8) == Piece::WHITE_KNIGHT);

        // BISHOP
        pos_copy = pos;
        is_valid = (pos_copy.doMove("a7a8b") == core::DoMoveResult::LEGAL);
        CHECK(is_valid);
        CHECK(pos_copy.pieceOn(Square::A7) == Piece::NO_PIECE);
        CHECK(pos_copy.pieceOn(Square::A8) == Piece::WHITE_BISHOP);

        // ROOK
        pos_copy = pos;
        is_valid = (pos_copy.doMove("a7a8r") == core::DoMoveResult::LEGAL);
        CHECK(is_valid);
        CHECK(pos_copy.pieceOn(Square::A7) == Piece::NO_PIECE);
        CHECK(pos_copy.pieceOn(Square::A8) == Piece::WHITE_ROOK);

        // QUEEN
        pos_copy = pos;
        is_valid = (pos_copy.doMove("a7a8q") == core::DoMoveResult::LEGAL);
        CHECK(is_valid);
        CHECK(pos_copy.pieceOn(Square::A7) == Piece::NO_PIECE);
        CHECK(pos_copy.pieceOn(Square::A8) == Piece::WHITE_QUEEN);

        // MOVE_CAPTURE_PROMOTION

        // KNIGHT
        pos_copy = pos;
        is_valid = (pos_copy.doMove("a7b8n") == core::DoMoveResult::LEGAL);
        CHECK(is_valid);
        CHECK(pos_copy.pieceOn(Square::A7) == Piece::NO_PIECE);
        CHECK(pos_copy.pieceOn(Square::B8) == Piece::WHITE_KNIGHT);

        // BISHOP
        pos_copy = pos;
        is_valid = (pos_copy.doMove("a7b8b") == core::DoMoveResult::LEGAL);
        CHECK(is_valid);
        CHECK(pos_copy.pieceOn(Square::A7) == Piece::NO_PIECE);
        CHECK(pos_copy.pieceOn(Square::B8) == Piece::WHITE_BISHOP);

        // ROOK
        pos_copy = pos;
        is_valid = (pos_copy.doMove("a7b8r") == core::DoMoveResult::LEGAL);
        CHECK(is_valid);
        CHECK(pos_copy.pieceOn(Square::A7) == Piece::NO_PIECE);
        CHECK(pos_copy.pieceOn(Square::B8) == Piece::WHITE_ROOK);

        // QUEEN
        pos_copy = pos;
        is_valid = (pos_copy.doMove("a7b8q") == core::DoMoveResult::LEGAL);
        CHECK(is_valid);
        CHECK(pos_copy.pieceOn(Square::A7) == Piece::NO_PIECE);
        CHECK(pos_copy.pieceOn(Square::B8) == Piece::WHITE_QUEEN);
    }

    TEST_CASE("Position::isDrawn") {
        std::vector<u64> key_history;

        core::Position pos;
        pos.setFen("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");

        const u64 startpos_hash = pos.key();

        key_history.push_back(pos.key());
        bool is_valid = (pos.doMove("g1f3") == core::DoMoveResult::LEGAL);
        CHECK(is_valid);
        CHECK(pos.isDrawn(key_history) == false);
        CHECK(pos.stm() == Color::BLACK);

        key_history.push_back(pos.key());
        is_valid = (pos.doMove("g8f6") == core::DoMoveResult::LEGAL);
        CHECK(is_valid);
        CHECK(pos.isDrawn(key_history) == false);
        CHECK(pos.stm() == Color::WHITE);

        key_history.push_back(pos.key());
        is_valid = (pos.doMove("f3g1") == core::DoMoveResult::LEGAL);
        CHECK(is_valid);
        CHECK(pos.isDrawn(key_history) == false);
        CHECK(pos.stm() == Color::BLACK);

        key_history.push_back(pos.key());
        is_valid = (pos.doMove("f6g8") == core::DoMoveResult::LEGAL);
        CHECK(is_valid);
        CHECK(pos.isDrawn(key_history) == true);
        CHECK(pos.stm() == Color::WHITE);

        key_history.push_back(pos.key());
        is_valid = (pos.doMove("g1f3") == core::DoMoveResult::LEGAL);
        CHECK(is_valid);
        CHECK(pos.isDrawn(key_history) == true);
        CHECK(pos.stm() == Color::BLACK);

        key_history.push_back(pos.key());
        is_valid = (pos.doMove("g8f6") == core::DoMoveResult::LEGAL);
        CHECK(is_valid);
        CHECK(pos.isDrawn(key_history) == true);
        CHECK(pos.stm() == Color::WHITE);

        key_history.push_back(pos.key());
        is_valid = (pos.doMove("f3g1") == core::DoMoveResult::LEGAL);
        CHECK(is_valid);
        CHECK(pos.isDrawn(key_history) == true);
        CHECK(pos.stm() == Color::BLACK);

        key_history.push_back(pos.key());
        is_valid = (pos.doMove("f6g8") == core::DoMoveResult::LEGAL);
        CHECK(is_valid);
        CHECK(pos.isDrawn(key_history) == true);
        CHECK(pos.stm() == Color::WHITE);

        CHECK(pos.key() == startpos_hash);
    }
}
