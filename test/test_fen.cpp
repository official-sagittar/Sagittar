#include "doctest/doctest.h"
#include "fen.h"
#include "pch.h"
#include "position.h"
#include "types.h"

using namespace sagittar;

TEST_SUITE("FEN") {

    TEST_CASE("fen::parseFEN") {
        core::Position pos;

        std::string startpos_fen = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";
        fen::parseFEN(&pos, startpos_fen);

        core::Position startpos;
        startpos.setStartpos();

        CHECK(pos == startpos);

        startpos_fen = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq -";
        fen::parseFEN(&pos, startpos_fen);

        CHECK(pos == startpos);

        startpos_fen = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - \"1/2-1/2\"";
        fen::parseFEN(&pos, startpos_fen, false);

        CHECK(pos == startpos);

        // Test with Kiwipete
        std::string fen = "r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq -";
        fen::parseFEN(&pos, fen);

        REQUIRE(pos.isValid());

        REQUIRE(pos.getPiece(Square::A1) == Piece::WHITE_ROOK);
        REQUIRE(pos.getPiece(Square::B1) == Piece::NO_PIECE);
        REQUIRE(pos.getPiece(Square::C1) == Piece::NO_PIECE);
        REQUIRE(pos.getPiece(Square::D1) == Piece::NO_PIECE);
        REQUIRE(pos.getPiece(Square::E1) == Piece::WHITE_KING);
        REQUIRE(pos.getPiece(Square::F1) == Piece::NO_PIECE);
        REQUIRE(pos.getPiece(Square::G1) == Piece::NO_PIECE);
        REQUIRE(pos.getPiece(Square::H1) == Piece::WHITE_ROOK);

        REQUIRE(pos.getPiece(Square::A2) == Piece::WHITE_PAWN);
        REQUIRE(pos.getPiece(Square::B2) == Piece::WHITE_PAWN);
        REQUIRE(pos.getPiece(Square::C2) == Piece::WHITE_PAWN);
        REQUIRE(pos.getPiece(Square::D2) == Piece::WHITE_BISHOP);
        REQUIRE(pos.getPiece(Square::E2) == Piece::WHITE_BISHOP);
        REQUIRE(pos.getPiece(Square::F2) == Piece::WHITE_PAWN);
        REQUIRE(pos.getPiece(Square::G2) == Piece::WHITE_PAWN);
        REQUIRE(pos.getPiece(Square::H2) == Piece::WHITE_PAWN);

        REQUIRE(pos.getPiece(Square::A3) == Piece::NO_PIECE);
        REQUIRE(pos.getPiece(Square::B3) == Piece::NO_PIECE);
        REQUIRE(pos.getPiece(Square::C3) == Piece::WHITE_KNIGHT);
        REQUIRE(pos.getPiece(Square::D3) == Piece::NO_PIECE);
        REQUIRE(pos.getPiece(Square::E3) == Piece::NO_PIECE);
        REQUIRE(pos.getPiece(Square::F3) == Piece::WHITE_QUEEN);
        REQUIRE(pos.getPiece(Square::G3) == Piece::NO_PIECE);
        REQUIRE(pos.getPiece(Square::H3) == Piece::BLACK_PAWN);

        REQUIRE(pos.getPiece(Square::A4) == Piece::NO_PIECE);
        REQUIRE(pos.getPiece(Square::B4) == Piece::BLACK_PAWN);
        REQUIRE(pos.getPiece(Square::C4) == Piece::NO_PIECE);
        REQUIRE(pos.getPiece(Square::D4) == Piece::NO_PIECE);
        REQUIRE(pos.getPiece(Square::E4) == Piece::WHITE_PAWN);
        REQUIRE(pos.getPiece(Square::F4) == Piece::NO_PIECE);
        REQUIRE(pos.getPiece(Square::G4) == Piece::NO_PIECE);
        REQUIRE(pos.getPiece(Square::H4) == Piece::NO_PIECE);

        REQUIRE(pos.getPiece(Square::A5) == Piece::NO_PIECE);
        REQUIRE(pos.getPiece(Square::B5) == Piece::NO_PIECE);
        REQUIRE(pos.getPiece(Square::C5) == Piece::NO_PIECE);
        REQUIRE(pos.getPiece(Square::D5) == Piece::WHITE_PAWN);
        REQUIRE(pos.getPiece(Square::E5) == Piece::WHITE_KNIGHT);
        REQUIRE(pos.getPiece(Square::F5) == Piece::NO_PIECE);
        REQUIRE(pos.getPiece(Square::G5) == Piece::NO_PIECE);
        REQUIRE(pos.getPiece(Square::H5) == Piece::NO_PIECE);

        REQUIRE(pos.getPiece(Square::A6) == Piece::BLACK_BISHOP);
        REQUIRE(pos.getPiece(Square::B6) == Piece::BLACK_KNIGHT);
        REQUIRE(pos.getPiece(Square::C6) == Piece::NO_PIECE);
        REQUIRE(pos.getPiece(Square::D6) == Piece::NO_PIECE);
        REQUIRE(pos.getPiece(Square::E6) == Piece::BLACK_PAWN);
        REQUIRE(pos.getPiece(Square::F6) == Piece::BLACK_KNIGHT);
        REQUIRE(pos.getPiece(Square::G6) == Piece::BLACK_PAWN);
        REQUIRE(pos.getPiece(Square::H6) == Piece::NO_PIECE);

        REQUIRE(pos.getPiece(Square::A7) == Piece::BLACK_PAWN);
        REQUIRE(pos.getPiece(Square::B7) == Piece::NO_PIECE);
        REQUIRE(pos.getPiece(Square::C7) == Piece::BLACK_PAWN);
        REQUIRE(pos.getPiece(Square::D7) == Piece::BLACK_PAWN);
        REQUIRE(pos.getPiece(Square::E7) == Piece::BLACK_QUEEN);
        REQUIRE(pos.getPiece(Square::F7) == Piece::BLACK_PAWN);
        REQUIRE(pos.getPiece(Square::G7) == Piece::BLACK_BISHOP);
        REQUIRE(pos.getPiece(Square::H7) == Piece::NO_PIECE);

        REQUIRE(pos.getPiece(Square::A8) == Piece::BLACK_ROOK);
        REQUIRE(pos.getPiece(Square::B8) == Piece::NO_PIECE);
        REQUIRE(pos.getPiece(Square::C8) == Piece::NO_PIECE);
        REQUIRE(pos.getPiece(Square::D8) == Piece::NO_PIECE);
        REQUIRE(pos.getPiece(Square::E8) == Piece::BLACK_KING);
        REQUIRE(pos.getPiece(Square::F8) == Piece::NO_PIECE);
        REQUIRE(pos.getPiece(Square::G8) == Piece::NO_PIECE);
        REQUIRE(pos.getPiece(Square::H8) == Piece::BLACK_ROOK);

        REQUIRE(pos.getActiveColor() == Color::WHITE);
        REQUIRE((pos.getCastelingRights() & core::CastleFlag::WKCA) != 0);
        REQUIRE((pos.getCastelingRights() & core::CastleFlag::WQCA) != 0);
        REQUIRE((pos.getCastelingRights() & core::CastleFlag::BKCA) != 0);
        REQUIRE((pos.getCastelingRights() & core::CastleFlag::BQCA) != 0);
        REQUIRE(pos.getEnpassantTarget() == Square::NO_SQ);
        REQUIRE(pos.getHalfmoveClock() == 0);
        REQUIRE(pos.getFullmoveNumber() == 1);
    }
}
