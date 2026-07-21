#include "commons/pch.h"
#include "core/position.h"
#include "core/types.h"
#include "doctest/doctest.h"

using namespace sagittar;

TEST_SUITE("FEN") {

    TEST_CASE("fen::parseFEN") {
        std::string startpos_fen = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";
        Position    startpos;
        startpos.setFen(startpos_fen);

        Position pos;
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

        REQUIRE(pos.pieceOn(Square{Square::Raw::A1}) == Piece::WHITE_ROOK);
        REQUIRE(pos.pieceOn(Square{Square::Raw::B1}) == Piece::NO_PIECE);
        REQUIRE(pos.pieceOn(Square{Square::Raw::C1}) == Piece::NO_PIECE);
        REQUIRE(pos.pieceOn(Square{Square::Raw::D1}) == Piece::NO_PIECE);
        REQUIRE(pos.pieceOn(Square{Square::Raw::E1}) == Piece::WHITE_KING);
        REQUIRE(pos.pieceOn(Square{Square::Raw::F1}) == Piece::NO_PIECE);
        REQUIRE(pos.pieceOn(Square{Square::Raw::G1}) == Piece::NO_PIECE);
        REQUIRE(pos.pieceOn(Square{Square::Raw::H1}) == Piece::WHITE_ROOK);

        REQUIRE(pos.pieceOn(Square{Square::Raw::A2}) == Piece::WHITE_PAWN);
        REQUIRE(pos.pieceOn(Square{Square::Raw::B2}) == Piece::WHITE_PAWN);
        REQUIRE(pos.pieceOn(Square{Square::Raw::C2}) == Piece::WHITE_PAWN);
        REQUIRE(pos.pieceOn(Square{Square::Raw::D2}) == Piece::WHITE_BISHOP);
        REQUIRE(pos.pieceOn(Square{Square::Raw::E2}) == Piece::WHITE_BISHOP);
        REQUIRE(pos.pieceOn(Square{Square::Raw::F2}) == Piece::WHITE_PAWN);
        REQUIRE(pos.pieceOn(Square{Square::Raw::G2}) == Piece::WHITE_PAWN);
        REQUIRE(pos.pieceOn(Square{Square::Raw::H2}) == Piece::WHITE_PAWN);

        REQUIRE(pos.pieceOn(Square{Square::Raw::A3}) == Piece::NO_PIECE);
        REQUIRE(pos.pieceOn(Square{Square::Raw::B3}) == Piece::NO_PIECE);
        REQUIRE(pos.pieceOn(Square{Square::Raw::C3}) == Piece::WHITE_KNIGHT);
        REQUIRE(pos.pieceOn(Square{Square::Raw::D3}) == Piece::NO_PIECE);
        REQUIRE(pos.pieceOn(Square{Square::Raw::E3}) == Piece::NO_PIECE);
        REQUIRE(pos.pieceOn(Square{Square::Raw::F3}) == Piece::WHITE_QUEEN);
        REQUIRE(pos.pieceOn(Square{Square::Raw::G3}) == Piece::NO_PIECE);
        REQUIRE(pos.pieceOn(Square{Square::Raw::H3}) == Piece::BLACK_PAWN);

        REQUIRE(pos.pieceOn(Square{Square::Raw::A4}) == Piece::NO_PIECE);
        REQUIRE(pos.pieceOn(Square{Square::Raw::B4}) == Piece::BLACK_PAWN);
        REQUIRE(pos.pieceOn(Square{Square::Raw::C4}) == Piece::NO_PIECE);
        REQUIRE(pos.pieceOn(Square{Square::Raw::D4}) == Piece::NO_PIECE);
        REQUIRE(pos.pieceOn(Square{Square::Raw::E4}) == Piece::WHITE_PAWN);
        REQUIRE(pos.pieceOn(Square{Square::Raw::F4}) == Piece::NO_PIECE);
        REQUIRE(pos.pieceOn(Square{Square::Raw::G4}) == Piece::NO_PIECE);
        REQUIRE(pos.pieceOn(Square{Square::Raw::H4}) == Piece::NO_PIECE);

        REQUIRE(pos.pieceOn(Square{Square::Raw::A5}) == Piece::NO_PIECE);
        REQUIRE(pos.pieceOn(Square{Square::Raw::B5}) == Piece::NO_PIECE);
        REQUIRE(pos.pieceOn(Square{Square::Raw::C5}) == Piece::NO_PIECE);
        REQUIRE(pos.pieceOn(Square{Square::Raw::D5}) == Piece::WHITE_PAWN);
        REQUIRE(pos.pieceOn(Square{Square::Raw::E5}) == Piece::WHITE_KNIGHT);
        REQUIRE(pos.pieceOn(Square{Square::Raw::F5}) == Piece::NO_PIECE);
        REQUIRE(pos.pieceOn(Square{Square::Raw::G5}) == Piece::NO_PIECE);
        REQUIRE(pos.pieceOn(Square{Square::Raw::H5}) == Piece::NO_PIECE);

        REQUIRE(pos.pieceOn(Square{Square::Raw::A6}) == Piece::BLACK_BISHOP);
        REQUIRE(pos.pieceOn(Square{Square::Raw::B6}) == Piece::BLACK_KNIGHT);
        REQUIRE(pos.pieceOn(Square{Square::Raw::C6}) == Piece::NO_PIECE);
        REQUIRE(pos.pieceOn(Square{Square::Raw::D6}) == Piece::NO_PIECE);
        REQUIRE(pos.pieceOn(Square{Square::Raw::E6}) == Piece::BLACK_PAWN);
        REQUIRE(pos.pieceOn(Square{Square::Raw::F6}) == Piece::BLACK_KNIGHT);
        REQUIRE(pos.pieceOn(Square{Square::Raw::G6}) == Piece::BLACK_PAWN);
        REQUIRE(pos.pieceOn(Square{Square::Raw::H6}) == Piece::NO_PIECE);

        REQUIRE(pos.pieceOn(Square{Square::Raw::A7}) == Piece::BLACK_PAWN);
        REQUIRE(pos.pieceOn(Square{Square::Raw::B7}) == Piece::NO_PIECE);
        REQUIRE(pos.pieceOn(Square{Square::Raw::C7}) == Piece::BLACK_PAWN);
        REQUIRE(pos.pieceOn(Square{Square::Raw::D7}) == Piece::BLACK_PAWN);
        REQUIRE(pos.pieceOn(Square{Square::Raw::E7}) == Piece::BLACK_QUEEN);
        REQUIRE(pos.pieceOn(Square{Square::Raw::F7}) == Piece::BLACK_PAWN);
        REQUIRE(pos.pieceOn(Square{Square::Raw::G7}) == Piece::BLACK_BISHOP);
        REQUIRE(pos.pieceOn(Square{Square::Raw::H7}) == Piece::NO_PIECE);

        REQUIRE(pos.pieceOn(Square{Square::Raw::A8}) == Piece::BLACK_ROOK);
        REQUIRE(pos.pieceOn(Square{Square::Raw::B8}) == Piece::NO_PIECE);
        REQUIRE(pos.pieceOn(Square{Square::Raw::C8}) == Piece::NO_PIECE);
        REQUIRE(pos.pieceOn(Square{Square::Raw::D8}) == Piece::NO_PIECE);
        REQUIRE(pos.pieceOn(Square{Square::Raw::E8}) == Piece::BLACK_KING);
        REQUIRE(pos.pieceOn(Square{Square::Raw::F8}) == Piece::NO_PIECE);
        REQUIRE(pos.pieceOn(Square{Square::Raw::G8}) == Piece::NO_PIECE);
        REQUIRE(pos.pieceOn(Square{Square::Raw::H8}) == Piece::BLACK_ROOK);

        REQUIRE(pos.stm() == Color::WHITE);
        REQUIRE((pos.caRights() & CastleFlag::WKCA) != 0);
        REQUIRE((pos.caRights() & CastleFlag::WQCA) != 0);
        REQUIRE((pos.caRights() & CastleFlag::BKCA) != 0);
        REQUIRE((pos.caRights() & CastleFlag::BQCA) != 0);
        REQUIRE(pos.epTarget() == Square{Square::Raw::NONE});
        REQUIRE(pos.halfmoves() == 0);
        REQUIRE(pos.fullmoves() == 1);
    }
}
