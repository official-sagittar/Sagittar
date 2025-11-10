#pragma once

#include "commons/pch.h"
#include "core/types.h"

namespace sagittar {

    using BitBoard = u64;

    inline constexpr BitBoard BB(const Square sq) { return static_cast<BitBoard>(1ULL << sq); }
    inline constexpr BitBoard BB(const int sq) { return static_cast<BitBoard>(1ULL << sq); }

    inline constexpr BitBoard MASK64(const bool x) { return -static_cast<BitBoard>(x); }

    inline constexpr BitBoard RANK_1_BB       = 0xFFULL;
    inline constexpr BitBoard RANK_2_BB       = RANK_1_BB << (8 * 1);
    inline constexpr BitBoard RANK_3_BB       = RANK_1_BB << (8 * 2);
    inline constexpr BitBoard RANK_4_BB       = RANK_1_BB << (8 * 3);
    inline constexpr BitBoard RANK_5_BB       = RANK_1_BB << (8 * 4);
    inline constexpr BitBoard RANK_6_BB       = RANK_1_BB << (8 * 5);
    inline constexpr BitBoard RANK_7_BB       = RANK_1_BB << (8 * 6);
    inline constexpr BitBoard RANK_8_BB       = RANK_1_BB << (8 * 7);
    inline constexpr BitBoard RANK_1_AND_8_BB = RANK_1_BB & RANK_8_BB;

    inline constexpr BitBoard FILE_A_BB = 0x0101010101010101ULL;
    inline constexpr BitBoard FILE_B_BB = FILE_A_BB << 1;
    inline constexpr BitBoard FILE_C_BB = FILE_A_BB << 2;
    inline constexpr BitBoard FILE_D_BB = FILE_A_BB << 3;
    inline constexpr BitBoard FILE_E_BB = FILE_A_BB << 4;
    inline constexpr BitBoard FILE_F_BB = FILE_A_BB << 5;
    inline constexpr BitBoard FILE_G_BB = FILE_A_BB << 6;
    inline constexpr BitBoard FILE_H_BB = FILE_A_BB << 7;

    inline constexpr BitBoard RANK_BB(const Rank r) { return RANK_1_BB << (8 * r); }
    inline constexpr BitBoard FILE_BB(const File f) { return FILE_A_BB << f; }

    enum Direction : i8 {
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

    template<Direction D>
    inline constexpr BitBoard shift(const BitBoard b) {
        return D == Direction::NORTH      ? b << 8
             : D == Direction::SOUTH      ? b >> 8
             : D == Direction::NORTH_2X   ? b << 16
             : D == Direction::SOUTH_2X   ? b >> 16
             : D == Direction::EAST       ? (b & ~FILE_H_BB) << 1
             : D == Direction::WEST       ? (b & ~FILE_A_BB) >> 1
             : D == Direction::NORTH_EAST ? (b & ~FILE_H_BB) << 9
             : D == Direction::NORTH_WEST ? (b & ~FILE_A_BB) << 7
             : D == Direction::SOUTH_EAST ? (b & ~FILE_H_BB) >> 7
             : D == Direction::SOUTH_WEST ? (b & ~FILE_A_BB) >> 9
                                          : 0ULL;
    }

}
