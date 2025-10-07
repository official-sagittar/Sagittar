#pragma once

#include "core/defs.h"

namespace sagittar {

    namespace core {

        enum Direction : int8_t {
            NORTH = 8,
            EAST  = 1,
            SOUTH = -NORTH,
            WEST  = -EAST,

            NORTH_EAST = NORTH + EAST,
            NORTH_WEST = NORTH + WEST,
            SOUTH_EAST = SOUTH + EAST,
            SOUTH_WEST = SOUTH + WEST,

            NORTH_2X = NORTH + NORTH,
            SOUTH_2X = SOUTH + SOUTH,
        };

        constexpr BitBoard BITBOARD_MASK_RANK_1       = 0xFF;
        constexpr BitBoard BITBOARD_MASK_RANK_2       = BITBOARD_MASK_RANK_1 << (8 * 1);
        constexpr BitBoard BITBOARD_MASK_RANK_3       = BITBOARD_MASK_RANK_1 << (8 * 2);
        constexpr BitBoard BITBOARD_MASK_RANK_4       = BITBOARD_MASK_RANK_1 << (8 * 3);
        constexpr BitBoard BITBOARD_MASK_RANK_5       = BITBOARD_MASK_RANK_1 << (8 * 4);
        constexpr BitBoard BITBOARD_MASK_RANK_6       = BITBOARD_MASK_RANK_1 << (8 * 5);
        constexpr BitBoard BITBOARD_MASK_RANK_7       = BITBOARD_MASK_RANK_1 << (8 * 6);
        constexpr BitBoard BITBOARD_MASK_RANK_8       = BITBOARD_MASK_RANK_1 << (8 * 7);
        constexpr BitBoard BITBOARD_MASK_NOT_RANK_1   = ~BITBOARD_MASK_RANK_1;
        constexpr BitBoard BITBOARD_MASK_NOT_RANK_8   = ~BITBOARD_MASK_RANK_8;
        constexpr BitBoard BITBOARD_MASK_RANK_1_AND_8 = BITBOARD_MASK_RANK_1 & BITBOARD_MASK_RANK_8;

        constexpr BitBoard BITBOARD_MASK_FILE_A = 0x0101010101010101ULL;
        constexpr BitBoard BITBOARD_MASK_FILE_B = BITBOARD_MASK_FILE_A << 1;
        constexpr BitBoard BITBOARD_MASK_FILE_C = BITBOARD_MASK_FILE_A << 2;
        constexpr BitBoard BITBOARD_MASK_FILE_D = BITBOARD_MASK_FILE_A << 3;
        constexpr BitBoard BITBOARD_MASK_FILE_E = BITBOARD_MASK_FILE_A << 4;
        constexpr BitBoard BITBOARD_MASK_FILE_F = BITBOARD_MASK_FILE_A << 5;
        constexpr BitBoard BITBOARD_MASK_FILE_G = BITBOARD_MASK_FILE_A << 6;
        constexpr BitBoard BITBOARD_MASK_FILE_H = BITBOARD_MASK_FILE_A << 7;

        template<Color US>
        constexpr int EP_VICTIM_SQUARE_DIR = (US == WHITE) ? 8 : -8;

        template<Direction D>
        constexpr BitBoard shift(const BitBoard b) {
            return D == Direction::NORTH      ? b << 8
                 : D == Direction::SOUTH      ? b >> 8
                 : D == Direction::NORTH_2X   ? b << 16
                 : D == Direction::SOUTH_2X   ? b >> 16
                 : D == Direction::EAST       ? (b & ~BITBOARD_MASK_FILE_H) << 1
                 : D == Direction::WEST       ? (b & ~BITBOARD_MASK_FILE_A) >> 1
                 : D == Direction::NORTH_EAST ? (b & ~BITBOARD_MASK_FILE_H) << 9
                 : D == Direction::NORTH_WEST ? (b & ~BITBOARD_MASK_FILE_A) << 7
                 : D == Direction::SOUTH_EAST ? (b & ~BITBOARD_MASK_FILE_H) >> 7
                 : D == Direction::SOUTH_WEST ? (b & ~BITBOARD_MASK_FILE_A) >> 9
                                              : 0ULL;
        }

        BitBoard between(const Square x, const Square y);
        BitBoard ray(const Square x, const Square y);
        BitBoard line(const Square x, const Square y);

    }

}
