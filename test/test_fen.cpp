#include "commons/pch.h"
#include "core/position.h"
#include "core/types.h"
#include "doctest/doctest.h"

using namespace sagittar;

TEST_SUITE("FEN") {

    TEST_CASE("fen::parseFEN") {
        std::string    startpos_fen = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";
        core::Position startpos;
        startpos.setFen(startpos_fen);

        core::Position pos;
        startpos_fen = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq -";
        pos.setFen(startpos_fen);
        CHECK(pos == startpos);

        startpos_fen = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - \"1/2-1/2\"";
        pos.setFen(startpos_fen, false);
        CHECK(pos == startpos);

        // Test with Kiwipete
        std::string fen = "r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq -";
        pos.setFen(fen);

        REQUIRE(pos.isValid());

        REQUIRE(pos.pieceOn(Square::A1) == Piece::WHITE_ROOK);
        REQUIRE(pos.pieceOn(Square::B1) == Piece::NO_PIECE);
        REQUIRE(pos.pieceOn(Square::C1) == Piece::NO_PIECE);
        REQUIRE(pos.pieceOn(Square::D1) == Piece::NO_PIECE);
        REQUIRE(pos.pieceOn(Square::E1) == Piece::WHITE_KING);
        REQUIRE(pos.pieceOn(Square::F1) == Piece::NO_PIECE);
        REQUIRE(pos.pieceOn(Square::G1) == Piece::NO_PIECE);
        REQUIRE(pos.pieceOn(Square::H1) == Piece::WHITE_ROOK);

        REQUIRE(pos.pieceOn(Square::A2) == Piece::WHITE_PAWN);
        REQUIRE(pos.pieceOn(Square::B2) == Piece::WHITE_PAWN);
        REQUIRE(pos.pieceOn(Square::C2) == Piece::WHITE_PAWN);
        REQUIRE(pos.pieceOn(Square::D2) == Piece::WHITE_BISHOP);
        REQUIRE(pos.pieceOn(Square::E2) == Piece::WHITE_BISHOP);
        REQUIRE(pos.pieceOn(Square::F2) == Piece::WHITE_PAWN);
        REQUIRE(pos.pieceOn(Square::G2) == Piece::WHITE_PAWN);
        REQUIRE(pos.pieceOn(Square::H2) == Piece::WHITE_PAWN);

        REQUIRE(pos.pieceOn(Square::A3) == Piece::NO_PIECE);
        REQUIRE(pos.pieceOn(Square::B3) == Piece::NO_PIECE);
        REQUIRE(pos.pieceOn(Square::C3) == Piece::WHITE_KNIGHT);
        REQUIRE(pos.pieceOn(Square::D3) == Piece::NO_PIECE);
        REQUIRE(pos.pieceOn(Square::E3) == Piece::NO_PIECE);
        REQUIRE(pos.pieceOn(Square::F3) == Piece::WHITE_QUEEN);
        REQUIRE(pos.pieceOn(Square::G3) == Piece::NO_PIECE);
        REQUIRE(pos.pieceOn(Square::H3) == Piece::BLACK_PAWN);

        REQUIRE(pos.pieceOn(Square::A4) == Piece::NO_PIECE);
        REQUIRE(pos.pieceOn(Square::B4) == Piece::BLACK_PAWN);
        REQUIRE(pos.pieceOn(Square::C4) == Piece::NO_PIECE);
        REQUIRE(pos.pieceOn(Square::D4) == Piece::NO_PIECE);
        REQUIRE(pos.pieceOn(Square::E4) == Piece::WHITE_PAWN);
        REQUIRE(pos.pieceOn(Square::F4) == Piece::NO_PIECE);
        REQUIRE(pos.pieceOn(Square::G4) == Piece::NO_PIECE);
        REQUIRE(pos.pieceOn(Square::H4) == Piece::NO_PIECE);

        REQUIRE(pos.pieceOn(Square::A5) == Piece::NO_PIECE);
        REQUIRE(pos.pieceOn(Square::B5) == Piece::NO_PIECE);
        REQUIRE(pos.pieceOn(Square::C5) == Piece::NO_PIECE);
        REQUIRE(pos.pieceOn(Square::D5) == Piece::WHITE_PAWN);
        REQUIRE(pos.pieceOn(Square::E5) == Piece::WHITE_KNIGHT);
        REQUIRE(pos.pieceOn(Square::F5) == Piece::NO_PIECE);
        REQUIRE(pos.pieceOn(Square::G5) == Piece::NO_PIECE);
        REQUIRE(pos.pieceOn(Square::H5) == Piece::NO_PIECE);

        REQUIRE(pos.pieceOn(Square::A6) == Piece::BLACK_BISHOP);
        REQUIRE(pos.pieceOn(Square::B6) == Piece::BLACK_KNIGHT);
        REQUIRE(pos.pieceOn(Square::C6) == Piece::NO_PIECE);
        REQUIRE(pos.pieceOn(Square::D6) == Piece::NO_PIECE);
        REQUIRE(pos.pieceOn(Square::E6) == Piece::BLACK_PAWN);
        REQUIRE(pos.pieceOn(Square::F6) == Piece::BLACK_KNIGHT);
        REQUIRE(pos.pieceOn(Square::G6) == Piece::BLACK_PAWN);
        REQUIRE(pos.pieceOn(Square::H6) == Piece::NO_PIECE);

        REQUIRE(pos.pieceOn(Square::A7) == Piece::BLACK_PAWN);
        REQUIRE(pos.pieceOn(Square::B7) == Piece::NO_PIECE);
        REQUIRE(pos.pieceOn(Square::C7) == Piece::BLACK_PAWN);
        REQUIRE(pos.pieceOn(Square::D7) == Piece::BLACK_PAWN);
        REQUIRE(pos.pieceOn(Square::E7) == Piece::BLACK_QUEEN);
        REQUIRE(pos.pieceOn(Square::F7) == Piece::BLACK_PAWN);
        REQUIRE(pos.pieceOn(Square::G7) == Piece::BLACK_BISHOP);
        REQUIRE(pos.pieceOn(Square::H7) == Piece::NO_PIECE);

        REQUIRE(pos.pieceOn(Square::A8) == Piece::BLACK_ROOK);
        REQUIRE(pos.pieceOn(Square::B8) == Piece::NO_PIECE);
        REQUIRE(pos.pieceOn(Square::C8) == Piece::NO_PIECE);
        REQUIRE(pos.pieceOn(Square::D8) == Piece::NO_PIECE);
        REQUIRE(pos.pieceOn(Square::E8) == Piece::BLACK_KING);
        REQUIRE(pos.pieceOn(Square::F8) == Piece::NO_PIECE);
        REQUIRE(pos.pieceOn(Square::G8) == Piece::NO_PIECE);
        REQUIRE(pos.pieceOn(Square::H8) == Piece::BLACK_ROOK);

        REQUIRE(pos.stm() == Color::WHITE);
        REQUIRE((pos.caRights() & core::CastleFlag::WKCA) != 0);
        REQUIRE((pos.caRights() & core::CastleFlag::WQCA) != 0);
        REQUIRE((pos.caRights() & core::CastleFlag::BKCA) != 0);
        REQUIRE((pos.caRights() & core::CastleFlag::BQCA) != 0);
        REQUIRE(pos.epTarget() == Square::NO_SQ);
        REQUIRE(pos.halfmoves() == 0);
        REQUIRE(pos.fullmoves() == 1);
    }
}
