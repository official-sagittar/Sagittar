#pragma once

#include "commons/pch.h"
#include "core/types.h"

namespace sagittar {

    class BitBoard {
       public:
        constexpr BitBoard() noexcept :
            m_bits(0ULL) { }

        constexpr BitBoard(const Square sq) noexcept :
            m_bits(1ULL << sq) { }

        constexpr BitBoard(const U64 bits) noexcept :
            m_bits(bits) { }

        constexpr BitBoard(const BitBoard& other) noexcept :
            m_bits(other.m_bits) { }

        constexpr BitBoard(BitBoard&& other) noexcept :
            m_bits(other.m_bits) { }

        inline constexpr BitBoard& operator=(const BitBoard& rhs) noexcept {
            if (this != &rhs)
            {
                m_bits = rhs.m_bits;
            }
            return *this;
        }

        inline constexpr BitBoard& operator=(const BitBoard&& rhs) noexcept {
            if (this != &rhs)
            {
                m_bits = rhs.m_bits;
            }
            return *this;
        }

        ~BitBoard() noexcept = default;

        [[nodiscard]] constexpr U64 raw() const noexcept { return m_bits; }

        [[nodiscard]] constexpr int count() const noexcept { return __builtin_popcountll(m_bits); }

        [[nodiscard]] constexpr bool isEmpty() const noexcept { return m_bits == 0ULL; }

        [[nodiscard]] constexpr bool isSingle() const noexcept { return count() == 1; }

        [[nodiscard]] constexpr bool hasMultiple() const noexcept { return count() > 1; }

        [[nodiscard]] constexpr int lsb() const noexcept { return __builtin_ctzll(m_bits); }

        [[nodiscard]] constexpr int popLsb() noexcept {
            const int position = lsb();
            m_bits &= m_bits - 1;
            return position;
        }

        constexpr BitBoard operator&(const U64 rhs) const noexcept {
            return BitBoard(m_bits & rhs);
        }

        constexpr BitBoard operator|(const U64 rhs) const noexcept {
            return BitBoard(m_bits | rhs);
        }

        constexpr BitBoard operator^(const U64 rhs) const noexcept {
            return BitBoard(m_bits ^ rhs);
        }

        constexpr BitBoard operator<<(const int rhs) const noexcept {
            return BitBoard(m_bits << rhs);
        }

        constexpr BitBoard operator>>(const int rhs) const noexcept {
            return BitBoard(m_bits >> rhs);
        }

        constexpr BitBoard operator&(const BitBoard& rhs) const noexcept {
            return BitBoard(m_bits & rhs.m_bits);
        }

        constexpr BitBoard operator|(const BitBoard& rhs) const noexcept {
            return BitBoard(m_bits | rhs.m_bits);
        }

        constexpr BitBoard operator^(const BitBoard& rhs) const noexcept {
            return BitBoard(m_bits ^ rhs.m_bits);
        }

        constexpr BitBoard operator~() const noexcept { return BitBoard(~m_bits); }

        constexpr BitBoard operator-() const noexcept { return BitBoard(-m_bits); }

        constexpr BitBoard operator-(const U64 rhs) const noexcept {
            return BitBoard(m_bits & ~rhs);
        }

        constexpr BitBoard operator-(const BitBoard& rhs) const noexcept {
            return BitBoard(m_bits & ~rhs.m_bits);
        }

        constexpr BitBoard& operator&=(const U64 rhs) noexcept {
            m_bits &= rhs;
            return *this;
        }

        constexpr BitBoard& operator|=(const U64 rhs) noexcept {
            m_bits |= rhs;
            return *this;
        }

        constexpr BitBoard& operator^=(const U64 rhs) noexcept {
            m_bits ^= rhs;
            return *this;
        }

        constexpr BitBoard& operator&=(const BitBoard& rhs) noexcept {
            m_bits &= rhs.m_bits;
            return *this;
        }

        constexpr BitBoard& operator|=(const BitBoard& rhs) noexcept {
            m_bits |= rhs.m_bits;
            return *this;
        }

        constexpr BitBoard& operator^=(const BitBoard& rhs) noexcept {
            m_bits ^= rhs.m_bits;
            return *this;
        }

        constexpr BitBoard& operator-=(const U64 rhs) noexcept {
            m_bits &= ~rhs;
            return *this;
        }

        constexpr BitBoard& operator-=(const BitBoard& rhs) noexcept {
            m_bits &= ~rhs.m_bits;
            return *this;
        }

        constexpr BitBoard& operator<<=(const int rhs) noexcept {
            m_bits <<= rhs;
            return *this;
        }

        constexpr BitBoard& operator>>=(const int rhs) noexcept {
            m_bits >>= rhs;
            return *this;
        }

        constexpr operator bool() const noexcept { return m_bits != 0ULL; }

        constexpr explicit operator U64() const noexcept { return m_bits; }

        constexpr bool operator==(const U64 rhs) const noexcept { return m_bits == rhs; }

        constexpr bool operator!=(const U64 rhs) const noexcept { return m_bits != rhs; }

        constexpr bool operator==(const BitBoard& rhs) const noexcept {
            return m_bits == rhs.m_bits;
        }

        constexpr bool operator!=(const BitBoard& rhs) const noexcept {
            return m_bits != rhs.m_bits;
        }

       private:
        U64 m_bits{};
    };

    static inline constexpr BitBoard bb(const Square sq) { return BitBoard(sq); }
    static inline constexpr BitBoard bb(const int sq) { return BitBoard(1ULL << sq); }

    static inline constexpr BitBoard k_RANK_1_BB(0xFFULL);
    static inline constexpr BitBoard k_RANK_2_BB       = k_RANK_1_BB << (8 * 1);
    static inline constexpr BitBoard k_RANK_3_BB       = k_RANK_1_BB << (8 * 2);
    static inline constexpr BitBoard k_RANK_4_BB       = k_RANK_1_BB << (8 * 3);
    static inline constexpr BitBoard k_RANK_5_BB       = k_RANK_1_BB << (8 * 4);
    static inline constexpr BitBoard k_RANK_6_BB       = k_RANK_1_BB << (8 * 5);
    static inline constexpr BitBoard k_RANK_7_BB       = k_RANK_1_BB << (8 * 6);
    static inline constexpr BitBoard k_RANK_8_BB       = k_RANK_1_BB << (8 * 7);
    static inline constexpr BitBoard k_RANK_1_AND_8_BB = k_RANK_1_BB | k_RANK_8_BB;

    static inline constexpr BitBoard k_FILE_A_BB(0x0101010101010101ULL);
    static inline constexpr BitBoard k_FILE_B_BB = k_FILE_A_BB << 1;
    static inline constexpr BitBoard k_FILE_C_BB = k_FILE_A_BB << 2;
    static inline constexpr BitBoard k_FILE_D_BB = k_FILE_A_BB << 3;
    static inline constexpr BitBoard k_FILE_E_BB = k_FILE_A_BB << 4;
    static inline constexpr BitBoard k_FILE_F_BB = k_FILE_A_BB << 5;
    static inline constexpr BitBoard k_FILE_G_BB = k_FILE_A_BB << 6;
    static inline constexpr BitBoard k_FILE_H_BB = k_FILE_A_BB << 7;

    static inline constexpr BitBoard rankBb(const Rank r) { return k_RANK_1_BB << (8 * r); }
    static inline constexpr BitBoard fileBb(const File f) { return k_FILE_A_BB << f; }

    enum Direction : I8 {
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
             : D == Direction::EAST       ? (b & ~k_FILE_H_BB) << 1
             : D == Direction::WEST       ? (b & ~k_FILE_A_BB) >> 1
             : D == Direction::NORTH_EAST ? (b & ~k_FILE_H_BB) << 9
             : D == Direction::NORTH_WEST ? (b & ~k_FILE_A_BB) << 7
             : D == Direction::SOUTH_EAST ? (b & ~k_FILE_H_BB) >> 7
             : D == Direction::SOUTH_WEST ? (b & ~k_FILE_A_BB) >> 9
                                          : BitBoard{};
    }

    inline constexpr bool isAligned(const Square x, const Square y) {
        const int rx = sq2rank(x), ry = sq2rank(y);
        const int fx = sq2file(x), fy = sq2file(y);
        return (rx == ry) || (fx == fy) || (rx - fx == ry - fy) || (rx + fx == ry + fy);
    }

    BitBoard ray(const Square, const Square);
    BitBoard line(const Square, const Square);
    BitBoard between(const Square, const Square);
}
