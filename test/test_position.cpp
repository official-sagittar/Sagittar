#include "doctest/doctest.h"
#include "fen.h"
#include "move.h"
#include "pch.h"
#include "position.h"
#include "types.h"

using namespace sagittar;

TEST_SUITE("Position") {
    TEST_CASE("Position::getPieceCount") {
        core::Position pos;
        pos.setStartpos();

        CHECK(pos.getPieceCount(Piece::WHITE_KING) == 1);
        CHECK(pos.getPieceCount(Piece::WHITE_QUEEN) == 1);
        CHECK(pos.getPieceCount(Piece::WHITE_ROOK) == 2);
        CHECK(pos.getPieceCount(Piece::WHITE_BISHOP) == 2);
        CHECK(pos.getPieceCount(Piece::WHITE_KNIGHT) == 2);
        CHECK(pos.getPieceCount(Piece::WHITE_PAWN) == 8);
        CHECK(pos.getPieceCount(Piece::WHITE_ROOK) == 2);

        CHECK(pos.getPieceCount(Piece::BLACK_KING) == 1);
        CHECK(pos.getPieceCount(Piece::BLACK_QUEEN) == 1);
        CHECK(pos.getPieceCount(Piece::BLACK_ROOK) == 2);
        CHECK(pos.getPieceCount(Piece::BLACK_BISHOP) == 2);
        CHECK(pos.getPieceCount(Piece::BLACK_KNIGHT) == 2);
        CHECK(pos.getPieceCount(Piece::BLACK_PAWN) == 8);
        CHECK(pos.getPieceCount(Piece::BLACK_ROOK) == 2);
    }

    TEST_CASE("Position::isValid") {
        core::Position pos;

        CHECK(pos.isValid() == false);

        pos.setStartpos();
        CHECK(pos.isValid() == true);

        pos.reset();

        pos.setPiece(Piece::WHITE_KING, Square::E1);
        CHECK(pos.isValid() == false);

        pos.setPiece(Piece::BLACK_KING, Square::E8);
        CHECK(pos.isValid() == true);

        pos.setPiece(Piece::WHITE_PAWN, Square::A1);
        CHECK(pos.isValid() == false);

        pos.clearPiece(Piece::WHITE_PAWN, Square::A1);
        CHECK(pos.isValid() == true);

        pos.setPiece(Piece::WHITE_PAWN, Square::H8);
        CHECK(pos.isValid() == false);

        pos.clearPiece(Piece::WHITE_PAWN, Square::H8);
        CHECK(pos.isValid() == true);

        pos.setPiece(Piece::BLACK_PAWN, Square::A8);
        CHECK(pos.isValid() == false);

        pos.clearPiece(Piece::BLACK_PAWN, Square::A8);
        CHECK(pos.isValid() == true);

        pos.setPiece(Piece::BLACK_PAWN, Square::H1);
        CHECK(pos.isValid() == false);

        pos.clearPiece(Piece::BLACK_PAWN, Square::H1);
        CHECK(pos.isValid() == true);
    }

    TEST_CASE("Position::movePiece and Position::undoMovePiece") {
        core::Position pos;

        SUBCASE("Quite Move") {
            pos.setStartpos();

            pos.movePiece(Piece::WHITE_PAWN, Square::E2, Square::E4);
            CHECK(pos.getPiece(Square::E2) == Piece::NO_PIECE);
            CHECK(pos.getPiece(Square::E4) == Piece::WHITE_PAWN);

            pos.undoMovePiece(Piece::WHITE_PAWN, Square::E2, Square::E4);
            CHECK(pos.getPiece(Square::E4) == Piece::NO_PIECE);
            CHECK(pos.getPiece(Square::E2) == Piece::WHITE_PAWN);

            pos.movePiece(Piece::WHITE_KNIGHT, Square::G1, Square::F3);
            CHECK(pos.getPiece(Square::G1) == Piece::NO_PIECE);
            CHECK(pos.getPiece(Square::F3) == Piece::WHITE_KNIGHT);

            pos.undoMovePiece(Piece::WHITE_KNIGHT, Square::G1, Square::F3);
            CHECK(pos.getPiece(Square::G1) == Piece::WHITE_KNIGHT);
            CHECK(pos.getPiece(Square::F3) == Piece::NO_PIECE);
        }

        SUBCASE("Capture + Promotion Move") {
            pos.reset();
            pos.setPiece(Piece::WHITE_KING, Square::E1);
            pos.setPiece(Piece::BLACK_KING, Square::E8);
            pos.setPiece(Piece::WHITE_PAWN, Square::A7);
            pos.setPiece(Piece::BLACK_QUEEN, Square::B8);
            CHECK(pos.isValid());

            pos.movePiece(Piece::WHITE_PAWN, Square::A7, Square::B8, true, true,
                          Piece::WHITE_QUEEN);
            CHECK(pos.getPiece(Square::E1) == Piece::WHITE_KING);
            CHECK(pos.getPiece(Square::E8) == Piece::BLACK_KING);
            CHECK(pos.getPiece(Square::A7) == Piece::NO_PIECE);
            CHECK(pos.getPiece(Square::B8) == Piece::WHITE_QUEEN);
            CHECK(pos.getPieceCount(WHITE_PAWN) == 0);
            CHECK(pos.getPieceCount(WHITE_QUEEN) == 1);
            CHECK(pos.getPieceCount(BLACK_QUEEN) == 0);

            pos.undoMovePiece(Piece::WHITE_PAWN, Square::A7, Square::B8, true, Piece::BLACK_QUEEN,
                              true, Piece::WHITE_QUEEN);
            CHECK(pos.getPiece(Square::E1) == Piece::WHITE_KING);
            CHECK(pos.getPiece(Square::E8) == Piece::BLACK_KING);
            CHECK(pos.getPiece(Square::A7) == Piece::WHITE_PAWN);
            CHECK(pos.getPiece(Square::B8) == Piece::BLACK_QUEEN);
            CHECK(pos.getPieceCount(WHITE_PAWN) == 1);
            CHECK(pos.getPieceCount(WHITE_QUEEN) == 0);
            CHECK(pos.getPieceCount(BLACK_QUEEN) == 1);
        }
    }

    TEST_CASE("Position::doMove::string") {
        core::Position pos;

        // MOVE_QUIET_PAWN_DBL_PUSH
        pos.setStartpos();
        bool is_valid = (pos.doMove("e2e4") == core::DoMoveResult::LEGAL);
        CHECK(is_valid);
        CHECK(pos.getPiece(Square::E2) == Piece::NO_PIECE);
        CHECK(pos.getPiece(Square::E4) == Piece::WHITE_PAWN);

        // MOVE_CASTLE_KING_SIDE WHITE
        std::string fen = "r1bqkb1r/pppp1ppp/2n2n2/4p3/2B1P3/5N2/PPPP1PPP/RNBQK2R w KQkq - 4 4";
        fen::parseFEN(&pos, fen);
        is_valid = (pos.doMove("e1g1") == core::DoMoveResult::LEGAL);
        CHECK(is_valid);
        CHECK(pos.getPiece(Square::E1) == Piece::NO_PIECE);
        CHECK(pos.getPiece(Square::F1) == Piece::WHITE_ROOK);
        CHECK(pos.getPiece(Square::G1) == Piece::WHITE_KING);
        CHECK(pos.getPiece(Square::H1) == Piece::NO_PIECE);

        // MOVE_CASTLE_KING_SIDE BLACK
        fen = "r1bqk2r/pppp1ppp/2n2n2/2b1p3/2B1P3/3P1N2/PPP2PPP/RNBQ1RK1 b kq - 0 5";
        fen::parseFEN(&pos, fen);
        is_valid = (pos.doMove("e8g8") == core::DoMoveResult::LEGAL);
        CHECK(is_valid);
        CHECK(pos.getPiece(Square::E8) == Piece::NO_PIECE);
        CHECK(pos.getPiece(Square::F8) == Piece::BLACK_ROOK);
        CHECK(pos.getPiece(Square::G8) == Piece::BLACK_KING);
        CHECK(pos.getPiece(Square::H8) == Piece::NO_PIECE);

        // MOVE_CASTLE_QUEEN_SIDE WHITE
        fen = "r3kb1r/pp1npppp/2p2n2/q4b2/3P1B2/2N2N2/PPPQ1PPP/R3KB1R w KQkq - 6 8";
        fen::parseFEN(&pos, fen);
        is_valid = (pos.doMove("e1c1") == core::DoMoveResult::LEGAL);
        CHECK(is_valid);
        CHECK(pos.getPiece(Square::E1) == Piece::NO_PIECE);
        CHECK(pos.getPiece(Square::D1) == Piece::WHITE_ROOK);
        CHECK(pos.getPiece(Square::C1) == Piece::WHITE_KING);
        CHECK(pos.getPiece(Square::B1) == Piece::NO_PIECE);
        CHECK(pos.getPiece(Square::A1) == Piece::NO_PIECE);

        // MOVE_CASTLE_QUEEN_SIDE BLACK
        fen = "r3kb1r/pp1npppp/2p2n2/q4b2/3P1B2/2N2N2/PPPQ1PPP/2KR1B1R b kq - 7 8";
        fen::parseFEN(&pos, fen);
        is_valid = (pos.doMove("e8c8") == core::DoMoveResult::LEGAL);
        CHECK(is_valid);
        CHECK(pos.getPiece(Square::E8) == Piece::NO_PIECE);
        CHECK(pos.getPiece(Square::D8) == Piece::BLACK_ROOK);
        CHECK(pos.getPiece(Square::C8) == Piece::BLACK_KING);
        CHECK(pos.getPiece(Square::B8) == Piece::NO_PIECE);
        CHECK(pos.getPiece(Square::A8) == Piece::NO_PIECE);

        // MOVE_CAPTURE_EP

        // WHITE
        fen = "4k3/8/8/3Pp3/8/8/8/4K3 w - e6 0 1";
        fen::parseFEN(&pos, fen);
        is_valid = (pos.doMove("d5e6") == core::DoMoveResult::LEGAL);
        CHECK(is_valid);
        CHECK(pos.getPiece(Square::D5) == Piece::NO_PIECE);
        CHECK(pos.getPiece(Square::E5) == Piece::NO_PIECE);
        CHECK(pos.getPiece(Square::E6) == Piece::WHITE_PAWN);

        // BLACK
        fen = "4k3/8/8/8/3pP3/8/8/4K3 b - e3 0 1";
        fen::parseFEN(&pos, fen);
        is_valid = (pos.doMove("d4e3") == core::DoMoveResult::LEGAL);
        CHECK(is_valid);
        CHECK(pos.getPiece(Square::D4) == Piece::NO_PIECE);
        CHECK(pos.getPiece(Square::E4) == Piece::NO_PIECE);
        CHECK(pos.getPiece(Square::E3) == Piece::BLACK_PAWN);

        // MOVE_PROMOTION
        fen = "1q2k3/P7/8/8/8/8/8/4K3 w - - 0 1";
        fen::parseFEN(&pos, fen);

        // KNIGHT
        core::Position pos_copy = pos;
        is_valid                = (pos_copy.doMove("a7a8n") == core::DoMoveResult::LEGAL);
        CHECK(is_valid);
        CHECK(pos_copy.getPiece(Square::A7) == Piece::NO_PIECE);
        CHECK(pos_copy.getPiece(Square::A8) == Piece::WHITE_KNIGHT);

        // BISHOP
        pos_copy = pos;
        is_valid = (pos_copy.doMove("a7a8b") == core::DoMoveResult::LEGAL);
        CHECK(is_valid);
        CHECK(pos_copy.getPiece(Square::A7) == Piece::NO_PIECE);
        CHECK(pos_copy.getPiece(Square::A8) == Piece::WHITE_BISHOP);

        // ROOK
        pos_copy = pos;
        is_valid = (pos_copy.doMove("a7a8r") == core::DoMoveResult::LEGAL);
        CHECK(is_valid);
        CHECK(pos_copy.getPiece(Square::A7) == Piece::NO_PIECE);
        CHECK(pos_copy.getPiece(Square::A8) == Piece::WHITE_ROOK);

        // QUEEN
        pos_copy = pos;
        is_valid = (pos_copy.doMove("a7a8q") == core::DoMoveResult::LEGAL);
        CHECK(is_valid);
        CHECK(pos_copy.getPiece(Square::A7) == Piece::NO_PIECE);
        CHECK(pos_copy.getPiece(Square::A8) == Piece::WHITE_QUEEN);

        // MOVE_CAPTURE_PROMOTION

        // KNIGHT
        pos_copy = pos;
        is_valid = (pos_copy.doMove("a7b8n") == core::DoMoveResult::LEGAL);
        CHECK(is_valid);
        CHECK(pos_copy.getPiece(Square::A7) == Piece::NO_PIECE);
        CHECK(pos_copy.getPiece(Square::B8) == Piece::WHITE_KNIGHT);

        // BISHOP
        pos_copy = pos;
        is_valid = (pos_copy.doMove("a7b8b") == core::DoMoveResult::LEGAL);
        CHECK(is_valid);
        CHECK(pos_copy.getPiece(Square::A7) == Piece::NO_PIECE);
        CHECK(pos_copy.getPiece(Square::B8) == Piece::WHITE_BISHOP);

        // ROOK
        pos_copy = pos;
        is_valid = (pos_copy.doMove("a7b8r") == core::DoMoveResult::LEGAL);
        CHECK(is_valid);
        CHECK(pos_copy.getPiece(Square::A7) == Piece::NO_PIECE);
        CHECK(pos_copy.getPiece(Square::B8) == Piece::WHITE_ROOK);

        // QUEEN
        pos_copy = pos;
        is_valid = (pos_copy.doMove("a7b8q") == core::DoMoveResult::LEGAL);
        CHECK(is_valid);
        CHECK(pos_copy.getPiece(Square::A7) == Piece::NO_PIECE);
        CHECK(pos_copy.getPiece(Square::B8) == Piece::WHITE_QUEEN);
    }

    TEST_CASE("Position::hasPositionRepeated") {
        std::vector<u64> key_history;

        core::Position pos;
        pos.setStartpos();

        const u64 startpos_hash = pos.getHash();

        key_history.push_back(pos.getHash());
        bool is_valid = (pos.doMove("g1f3") == core::DoMoveResult::LEGAL);
        CHECK(is_valid);
        CHECK(pos.hasPositionRepeated(key_history) == false);
        CHECK(pos.getActiveColor() == Color::BLACK);

        key_history.push_back(pos.getHash());
        is_valid = (pos.doMove("g8f6") == core::DoMoveResult::LEGAL);
        CHECK(is_valid);
        CHECK(pos.hasPositionRepeated(key_history) == false);
        CHECK(pos.getActiveColor() == Color::WHITE);

        key_history.push_back(pos.getHash());
        is_valid = (pos.doMove("f3g1") == core::DoMoveResult::LEGAL);
        CHECK(is_valid);
        CHECK(pos.hasPositionRepeated(key_history) == false);
        CHECK(pos.getActiveColor() == Color::BLACK);

        key_history.push_back(pos.getHash());
        is_valid = (pos.doMove("f6g8") == core::DoMoveResult::LEGAL);
        CHECK(is_valid);
        CHECK(pos.hasPositionRepeated(key_history) == true);
        CHECK(pos.getActiveColor() == Color::WHITE);

        key_history.push_back(pos.getHash());
        is_valid = (pos.doMove("g1f3") == core::DoMoveResult::LEGAL);
        CHECK(is_valid);
        CHECK(pos.hasPositionRepeated(key_history) == true);
        CHECK(pos.getActiveColor() == Color::BLACK);

        key_history.push_back(pos.getHash());
        is_valid = (pos.doMove("g8f6") == core::DoMoveResult::LEGAL);
        CHECK(is_valid);
        CHECK(pos.hasPositionRepeated(key_history) == true);
        CHECK(pos.getActiveColor() == Color::WHITE);

        key_history.push_back(pos.getHash());
        is_valid = (pos.doMove("f3g1") == core::DoMoveResult::LEGAL);
        CHECK(is_valid);
        CHECK(pos.hasPositionRepeated(key_history) == true);
        CHECK(pos.getActiveColor() == Color::BLACK);

        key_history.push_back(pos.getHash());
        is_valid = (pos.doMove("f6g8") == core::DoMoveResult::LEGAL);
        CHECK(is_valid);
        CHECK(pos.hasPositionRepeated(key_history) == true);
        CHECK(pos.getActiveColor() == Color::WHITE);

        CHECK(pos.getHash() == startpos_hash);
    }
}
