#pragma once

#include "pch.h"

namespace sagittar {

    namespace core {

        using BitBoard = uint64_t;
        using Score    = int16_t;

        enum Color {
            WHITE,
            BLACK
        };

        enum PieceType {
            PIECE_TYPE_INVALID,
            PAWN,
            KNIGHT,
            BISHOP,
            ROOK,
            QUEEN,
            KING
        };

        enum Piece {
            NO_PIECE,
            WHITE_PAWN,
            WHITE_KNIGHT,
            WHITE_BISHOP,
            WHITE_ROOK,
            WHITE_QUEEN,
            WHITE_KING,
            BLACK_PAWN = 9,
            BLACK_KNIGHT,
            BLACK_BISHOP,
            BLACK_ROOK,
            BLACK_QUEEN,
            BLACK_KING
        };

        enum Rank {
            RANK_1,
            RANK_2,
            RANK_3,
            RANK_4,
            RANK_5,
            RANK_6,
            RANK_7,
            RANK_8
        };

        enum File {
            FILE_A,
            FILE_B,
            FILE_C,
            FILE_D,
            FILE_E,
            FILE_F,
            FILE_G,
            FILE_H
        };

        // Little-Endian Rank-File Mapping
        // clang-format off
        enum Square {
            A1, B1, C1, D1, E1, F1, G1, H1,
            A2, B2, C2, D2, E2, F2, G2, H2,
            A3, B3, C3, D3, E3, F3, G3, H3,
            A4, B4, C4, D4, E4, F4, G4, H4,
            A5, B5, C5, D5, E5, F5, G5, H5,
            A6, B6, C6, D6, E6, F6, G6, H6,
            A7, B7, C7, D7, E7, F7, G7, H7,
            A8, B8, C8, D8, E8, F8, G8, H8
        };
        // clang-format on

        enum CastlingRights {
            NOCA = 0x0,
            WKCA = 0x1,
            WQCA = 0x2,
            BKCA = 0x4,
            BQCA = 0x8
        };

        inline constexpr Color COLOR_FLIP(const Color c) { return static_cast<Color>(c ^ 1); }

        inline constexpr Piece PIECE_CREATE(const PieceType pt, const Color c) {
            return static_cast<Piece>((c << 3) | pt);
        }
        inline constexpr PieceType PIECE_TYPE_OF(const Piece p) {
            return static_cast<PieceType>(p & 0x7);
        }
        inline constexpr Color PIECE_COLOR_OF(const Piece p) {
            return static_cast<Color>((p & 0x8) / 8);
        }

        inline constexpr Square RF_TO_SQ(const Rank r, const File f) {
            return static_cast<Square>(r * 8 + f);
        }
        inline constexpr Square RF_TO_SQ(const int r, const int f) {
            return static_cast<Square>(r * 8 + f);
        }
        inline constexpr Rank SQ_TO_RANK(const Square sq) { return static_cast<Rank>(sq >> 3); }
        inline constexpr Rank SQ_TO_RANK(const int sq) { return static_cast<Rank>(sq >> 3); }
        inline constexpr File SQ_TO_FILE(const Square sq) { return static_cast<File>(sq & 7); }
        inline constexpr File SQ_TO_FILE(const int sq) { return static_cast<File>(sq & 7); }

        inline constexpr BitBoard BB(const Square sq) { return static_cast<BitBoard>(1ULL << sq); }
        inline constexpr BitBoard BB(const int sq) { return static_cast<BitBoard>(1ULL << sq); }

        inline constexpr int POPCNT(const uint64_t x) { return __builtin_popcountll(x); }

        static const std::string PIECES_STR = ".PNBRQKXXpnbrqk";

    }

}
