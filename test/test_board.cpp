#include "board.h"
#include "doctest/doctest.h"
#include "fen.h"
#include "move.h"
#include "pch.h"
#include "types.h"

using namespace sagittar;

TEST_SUITE("Board") {
    TEST_CASE("Board::getPieceCount") {
        board::Board board;
        board.setStartpos();

        CHECK(board.getPieceCount(Piece::WHITE_KING) == 1);
        CHECK(board.getPieceCount(Piece::WHITE_QUEEN) == 1);
        CHECK(board.getPieceCount(Piece::WHITE_ROOK) == 2);
        CHECK(board.getPieceCount(Piece::WHITE_BISHOP) == 2);
        CHECK(board.getPieceCount(Piece::WHITE_KNIGHT) == 2);
        CHECK(board.getPieceCount(Piece::WHITE_PAWN) == 8);
        CHECK(board.getPieceCount(Piece::WHITE_ROOK) == 2);

        CHECK(board.getPieceCount(Piece::BLACK_KING) == 1);
        CHECK(board.getPieceCount(Piece::BLACK_QUEEN) == 1);
        CHECK(board.getPieceCount(Piece::BLACK_ROOK) == 2);
        CHECK(board.getPieceCount(Piece::BLACK_BISHOP) == 2);
        CHECK(board.getPieceCount(Piece::BLACK_KNIGHT) == 2);
        CHECK(board.getPieceCount(Piece::BLACK_PAWN) == 8);
        CHECK(board.getPieceCount(Piece::BLACK_ROOK) == 2);
    }

    TEST_CASE("Board::isValid") {
        board::Board board;

        CHECK(board.isValid() == false);

        board.setStartpos();
        CHECK(board.isValid() == true);

        board.reset();

        board.setPiece(Piece::WHITE_KING, Square::E1);
        CHECK(board.isValid() == false);

        board.setPiece(Piece::BLACK_KING, Square::E8);
        CHECK(board.isValid() == true);

        board.setPiece(Piece::WHITE_PAWN, Square::A1);
        CHECK(board.isValid() == false);

        board.clearPiece(Piece::WHITE_PAWN, Square::A1);
        CHECK(board.isValid() == true);

        board.setPiece(Piece::WHITE_PAWN, Square::H8);
        CHECK(board.isValid() == false);

        board.clearPiece(Piece::WHITE_PAWN, Square::H8);
        CHECK(board.isValid() == true);

        board.setPiece(Piece::BLACK_PAWN, Square::A8);
        CHECK(board.isValid() == false);

        board.clearPiece(Piece::BLACK_PAWN, Square::A8);
        CHECK(board.isValid() == true);

        board.setPiece(Piece::BLACK_PAWN, Square::H1);
        CHECK(board.isValid() == false);

        board.clearPiece(Piece::BLACK_PAWN, Square::H1);
        CHECK(board.isValid() == true);
    }

    TEST_CASE("Board::movePiece and Board::undoMovePiece") {
        board::Board board;

        SUBCASE("Quite Move") {
            board.setStartpos();

            board.movePiece(Piece::WHITE_PAWN, Square::E2, Square::E4);
            CHECK(board.getPiece(Square::E2) == Piece::NO_PIECE);
            CHECK(board.getPiece(Square::E4) == Piece::WHITE_PAWN);

            board.undoMovePiece(Piece::WHITE_PAWN, Square::E2, Square::E4);
            CHECK(board.getPiece(Square::E4) == Piece::NO_PIECE);
            CHECK(board.getPiece(Square::E2) == Piece::WHITE_PAWN);

            board.movePiece(Piece::WHITE_KNIGHT, Square::G1, Square::F3);
            CHECK(board.getPiece(Square::G1) == Piece::NO_PIECE);
            CHECK(board.getPiece(Square::F3) == Piece::WHITE_KNIGHT);

            board.undoMovePiece(Piece::WHITE_KNIGHT, Square::G1, Square::F3);
            CHECK(board.getPiece(Square::G1) == Piece::WHITE_KNIGHT);
            CHECK(board.getPiece(Square::F3) == Piece::NO_PIECE);
        }

        SUBCASE("Capture + Promotion Move") {
            board.reset();
            board.setPiece(Piece::WHITE_KING, Square::E1);
            board.setPiece(Piece::BLACK_KING, Square::E8);
            board.setPiece(Piece::WHITE_PAWN, Square::A7);
            board.setPiece(Piece::BLACK_QUEEN, Square::B8);
            CHECK(board.isValid());

            board.movePiece(Piece::WHITE_PAWN, Square::A7, Square::B8, true, true,
                            Piece::WHITE_QUEEN);
            CHECK(board.getPiece(Square::E1) == Piece::WHITE_KING);
            CHECK(board.getPiece(Square::E8) == Piece::BLACK_KING);
            CHECK(board.getPiece(Square::A7) == Piece::NO_PIECE);
            CHECK(board.getPiece(Square::B8) == Piece::WHITE_QUEEN);
            CHECK(board.getPieceCount(WHITE_PAWN) == 0);
            CHECK(board.getPieceCount(WHITE_QUEEN) == 1);
            CHECK(board.getPieceCount(BLACK_QUEEN) == 0);

            board.undoMovePiece(Piece::WHITE_PAWN, Square::A7, Square::B8, true, Piece::BLACK_QUEEN,
                                true, Piece::WHITE_QUEEN);
            CHECK(board.getPiece(Square::E1) == Piece::WHITE_KING);
            CHECK(board.getPiece(Square::E8) == Piece::BLACK_KING);
            CHECK(board.getPiece(Square::A7) == Piece::WHITE_PAWN);
            CHECK(board.getPiece(Square::B8) == Piece::BLACK_QUEEN);
            CHECK(board.getPieceCount(WHITE_PAWN) == 1);
            CHECK(board.getPieceCount(WHITE_QUEEN) == 0);
            CHECK(board.getPieceCount(BLACK_QUEEN) == 1);
        }
    }

    TEST_CASE("Board::doMove::string") {
        board::Board board;

        // MOVE_QUIET_PAWN_DBL_PUSH
        board.setStartpos();
        bool is_valid = (board.doMove("e2e4") == board::DoMoveResult::LEGAL);
        CHECK(is_valid);
        CHECK(board.getPiece(Square::E2) == Piece::NO_PIECE);
        CHECK(board.getPiece(Square::E4) == Piece::WHITE_PAWN);

        // MOVE_CASTLE_KING_SIDE WHITE
        std::string fen = "r1bqkb1r/pppp1ppp/2n2n2/4p3/2B1P3/5N2/PPPP1PPP/RNBQK2R w KQkq - 4 4";
        fen::parseFEN(&board, fen);
        is_valid = (board.doMove("e1g1") == board::DoMoveResult::LEGAL);
        CHECK(is_valid);
        CHECK(board.getPiece(Square::E1) == Piece::NO_PIECE);
        CHECK(board.getPiece(Square::F1) == Piece::WHITE_ROOK);
        CHECK(board.getPiece(Square::G1) == Piece::WHITE_KING);
        CHECK(board.getPiece(Square::H1) == Piece::NO_PIECE);

        // MOVE_CASTLE_KING_SIDE BLACK
        fen = "r1bqk2r/pppp1ppp/2n2n2/2b1p3/2B1P3/3P1N2/PPP2PPP/RNBQ1RK1 b kq - 0 5";
        fen::parseFEN(&board, fen);
        is_valid = (board.doMove("e8g8") == board::DoMoveResult::LEGAL);
        CHECK(is_valid);
        CHECK(board.getPiece(Square::E8) == Piece::NO_PIECE);
        CHECK(board.getPiece(Square::F8) == Piece::BLACK_ROOK);
        CHECK(board.getPiece(Square::G8) == Piece::BLACK_KING);
        CHECK(board.getPiece(Square::H8) == Piece::NO_PIECE);

        // MOVE_CASTLE_QUEEN_SIDE WHITE
        fen = "r3kb1r/pp1npppp/2p2n2/q4b2/3P1B2/2N2N2/PPPQ1PPP/R3KB1R w KQkq - 6 8";
        fen::parseFEN(&board, fen);
        is_valid = (board.doMove("e1c1") == board::DoMoveResult::LEGAL);
        CHECK(is_valid);
        CHECK(board.getPiece(Square::E1) == Piece::NO_PIECE);
        CHECK(board.getPiece(Square::D1) == Piece::WHITE_ROOK);
        CHECK(board.getPiece(Square::C1) == Piece::WHITE_KING);
        CHECK(board.getPiece(Square::B1) == Piece::NO_PIECE);
        CHECK(board.getPiece(Square::A1) == Piece::NO_PIECE);

        // MOVE_CASTLE_QUEEN_SIDE BLACK
        fen = "r3kb1r/pp1npppp/2p2n2/q4b2/3P1B2/2N2N2/PPPQ1PPP/2KR1B1R b kq - 7 8";
        fen::parseFEN(&board, fen);
        is_valid = (board.doMove("e8c8") == board::DoMoveResult::LEGAL);
        CHECK(is_valid);
        CHECK(board.getPiece(Square::E8) == Piece::NO_PIECE);
        CHECK(board.getPiece(Square::D8) == Piece::BLACK_ROOK);
        CHECK(board.getPiece(Square::C8) == Piece::BLACK_KING);
        CHECK(board.getPiece(Square::B8) == Piece::NO_PIECE);
        CHECK(board.getPiece(Square::A8) == Piece::NO_PIECE);

        // MOVE_CAPTURE_EP

        // WHITE
        fen = "4k3/8/8/3Pp3/8/8/8/4K3 w - e6 0 1";
        fen::parseFEN(&board, fen);
        is_valid = (board.doMove("d5e6") == board::DoMoveResult::LEGAL);
        CHECK(is_valid);
        CHECK(board.getPiece(Square::D5) == Piece::NO_PIECE);
        CHECK(board.getPiece(Square::E5) == Piece::NO_PIECE);
        CHECK(board.getPiece(Square::E6) == Piece::WHITE_PAWN);

        // BLACK
        fen = "4k3/8/8/8/3pP3/8/8/4K3 b - e3 0 1";
        fen::parseFEN(&board, fen);
        is_valid = (board.doMove("d4e3") == board::DoMoveResult::LEGAL);
        CHECK(is_valid);
        CHECK(board.getPiece(Square::D4) == Piece::NO_PIECE);
        CHECK(board.getPiece(Square::E4) == Piece::NO_PIECE);
        CHECK(board.getPiece(Square::E3) == Piece::BLACK_PAWN);

        // MOVE_PROMOTION
        fen = "1q2k3/P7/8/8/8/8/8/4K3 w - - 0 1";
        fen::parseFEN(&board, fen);

        // KNIGHT
        board::Board board_copy = board;
        is_valid                = (board_copy.doMove("a7a8n") == board::DoMoveResult::LEGAL);
        CHECK(is_valid);
        CHECK(board_copy.getPiece(Square::A7) == Piece::NO_PIECE);
        CHECK(board_copy.getPiece(Square::A8) == Piece::WHITE_KNIGHT);

        // BISHOP
        board_copy = board;
        is_valid   = (board_copy.doMove("a7a8b") == board::DoMoveResult::LEGAL);
        CHECK(is_valid);
        CHECK(board_copy.getPiece(Square::A7) == Piece::NO_PIECE);
        CHECK(board_copy.getPiece(Square::A8) == Piece::WHITE_BISHOP);

        // ROOK
        board_copy = board;
        is_valid   = (board_copy.doMove("a7a8r") == board::DoMoveResult::LEGAL);
        CHECK(is_valid);
        CHECK(board_copy.getPiece(Square::A7) == Piece::NO_PIECE);
        CHECK(board_copy.getPiece(Square::A8) == Piece::WHITE_ROOK);

        // QUEEN
        board_copy = board;
        is_valid   = (board_copy.doMove("a7a8q") == board::DoMoveResult::LEGAL);
        CHECK(is_valid);
        CHECK(board_copy.getPiece(Square::A7) == Piece::NO_PIECE);
        CHECK(board_copy.getPiece(Square::A8) == Piece::WHITE_QUEEN);

        // MOVE_CAPTURE_PROMOTION

        // KNIGHT
        board_copy = board;
        is_valid   = (board_copy.doMove("a7b8n") == board::DoMoveResult::LEGAL);
        CHECK(is_valid);
        CHECK(board_copy.getPiece(Square::A7) == Piece::NO_PIECE);
        CHECK(board_copy.getPiece(Square::B8) == Piece::WHITE_KNIGHT);

        // BISHOP
        board_copy = board;
        is_valid   = (board_copy.doMove("a7b8b") == board::DoMoveResult::LEGAL);
        CHECK(is_valid);
        CHECK(board_copy.getPiece(Square::A7) == Piece::NO_PIECE);
        CHECK(board_copy.getPiece(Square::B8) == Piece::WHITE_BISHOP);

        // ROOK
        board_copy = board;
        is_valid   = (board_copy.doMove("a7b8r") == board::DoMoveResult::LEGAL);
        CHECK(is_valid);
        CHECK(board_copy.getPiece(Square::A7) == Piece::NO_PIECE);
        CHECK(board_copy.getPiece(Square::B8) == Piece::WHITE_ROOK);

        // QUEEN
        board_copy = board;
        is_valid   = (board_copy.doMove("a7b8q") == board::DoMoveResult::LEGAL);
        CHECK(is_valid);
        CHECK(board_copy.getPiece(Square::A7) == Piece::NO_PIECE);
        CHECK(board_copy.getPiece(Square::B8) == Piece::WHITE_QUEEN);
    }

    TEST_CASE("Board::hasPositionRepeated") {
        std::vector<u64> key_history;

        board::Board board;
        board.setStartpos();

        const u64 startpos_hash = board.getHash();

        key_history.push_back(board.getHash());
        bool is_valid = (board.doMove("g1f3") == board::DoMoveResult::LEGAL);
        CHECK(is_valid);
        CHECK(board.hasPositionRepeated(key_history) == false);
        CHECK(board.getActiveColor() == Color::BLACK);

        key_history.push_back(board.getHash());
        is_valid = (board.doMove("g8f6") == board::DoMoveResult::LEGAL);
        CHECK(is_valid);
        CHECK(board.hasPositionRepeated(key_history) == false);
        CHECK(board.getActiveColor() == Color::WHITE);

        key_history.push_back(board.getHash());
        is_valid = (board.doMove("f3g1") == board::DoMoveResult::LEGAL);
        CHECK(is_valid);
        CHECK(board.hasPositionRepeated(key_history) == false);
        CHECK(board.getActiveColor() == Color::BLACK);

        key_history.push_back(board.getHash());
        is_valid = (board.doMove("f6g8") == board::DoMoveResult::LEGAL);
        CHECK(is_valid);
        CHECK(board.hasPositionRepeated(key_history) == true);
        CHECK(board.getActiveColor() == Color::WHITE);

        key_history.push_back(board.getHash());
        is_valid = (board.doMove("g1f3") == board::DoMoveResult::LEGAL);
        CHECK(is_valid);
        CHECK(board.hasPositionRepeated(key_history) == true);
        CHECK(board.getActiveColor() == Color::BLACK);

        key_history.push_back(board.getHash());
        is_valid = (board.doMove("g8f6") == board::DoMoveResult::LEGAL);
        CHECK(is_valid);
        CHECK(board.hasPositionRepeated(key_history) == true);
        CHECK(board.getActiveColor() == Color::WHITE);

        key_history.push_back(board.getHash());
        is_valid = (board.doMove("f3g1") == board::DoMoveResult::LEGAL);
        CHECK(is_valid);
        CHECK(board.hasPositionRepeated(key_history) == true);
        CHECK(board.getActiveColor() == Color::BLACK);

        key_history.push_back(board.getHash());
        is_valid = (board.doMove("f6g8") == board::DoMoveResult::LEGAL);
        CHECK(is_valid);
        CHECK(board.hasPositionRepeated(key_history) == true);
        CHECK(board.getActiveColor() == Color::WHITE);

        CHECK(board.getHash() == startpos_hash);
    }
}
