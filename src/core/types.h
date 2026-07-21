#pragma once

#include "commons/pch.h"

namespace sagittar {

    using u8   = std::uint8_t;
    using i8   = std::int8_t;
    using u16  = std::uint16_t;
    using i16  = std::int16_t;
    using u32  = std::uint32_t;
    using i32  = std::int32_t;
    using u64  = std::uint64_t;
    using u128 = unsigned __int128;

    using Score = i32;
    using Depth = i32;

    constexpr int MOVES_MAX = 256;

    enum Color : u8 {
        WHITE,
        BLACK
    };

    enum PieceType : u8 {
        PAWN,
        KNIGHT,
        BISHOP,
        ROOK,
        QUEEN,
        KING,
        PIECE_TYPE_INVALID,
    };

    enum Piece : u8 {
        WHITE_PAWN,
        WHITE_KNIGHT,
        WHITE_BISHOP,
        WHITE_ROOK,
        WHITE_QUEEN,
        WHITE_KING,
        BLACK_PAWN = 8,
        BLACK_KNIGHT,
        BLACK_BISHOP,
        BLACK_ROOK,
        BLACK_QUEEN,
        BLACK_KING,
        NO_PIECE
    };

    enum Rank : u8 {
        RANK_1,
        RANK_2,
        RANK_3,
        RANK_4,
        RANK_5,
        RANK_6,
        RANK_7,
        RANK_8
    };

    enum File : u8 {
        FILE_A,
        FILE_B,
        FILE_C,
        FILE_D,
        FILE_E,
        FILE_F,
        FILE_G,
        FILE_H
    };

    class Square final {
       public:
        static constexpr int N_SQUARES = 64;

        // Little-Endian Rank-File Mapping
        // clang-format off
        enum class Raw : std::uint8_t {
            A1, B1, C1, D1, E1, F1, G1, H1,
            A2, B2, C2, D2, E2, F2, G2, H2,
            A3, B3, C3, D3, E3, F3, G3, H3,
            A4, B4, C4, D4, E4, F4, G4, H4,
            A5, B5, C5, D5, E5, F5, G5, H5,
            A6, B6, C6, D6, E6, F6, G6, H6,
            A7, B7, C7, D7, E7, F7, G7, H7,
            A8, B8, C8, D8, E8, F8, G8, H8,
            NONE
        };
        // clang-format on

        constexpr Square() = default;
        constexpr Square(Raw value) :
            m_value(value) {}
        constexpr Square(int square) :
            m_value(static_cast<Raw>(square)) {
            assert(square >= 0 && square < 64);
        }
        constexpr Square(Rank r, File f) :
            m_value(static_cast<Raw>((8 * r) + f)) {}
        constexpr Square(int r, int f) :
            m_value(static_cast<Raw>((8 * r) + f)) {}

        [[nodiscard]] constexpr std::size_t index() const noexcept {
            return static_cast<std::size_t>(m_value);
        }
        [[nodiscard]] constexpr Raw raw() const noexcept { return m_value; }

        [[nodiscard]] constexpr Rank rank() const noexcept {
            return static_cast<Rank>(static_cast<int>(m_value) >> 3);
        }
        [[nodiscard]] constexpr File file() const noexcept {
            return static_cast<File>(static_cast<int>(m_value) & 7);
        }

        [[nodiscard]] constexpr Square flip() const noexcept {
            return Square{static_cast<int>(m_value) ^ 56};
        }

        [[nodiscard]] static constexpr auto all() noexcept {
            return std::views::iota(0, N_SQUARES)
                 | std::views::transform([](const int i) { return Square{i}; });
        }

        constexpr bool operator==(const Square& rhs) const noexcept {
            return m_value == rhs.m_value;
        }
        constexpr bool operator!=(const Square& rhs) const noexcept {
            return m_value != rhs.m_value;
        }
        constexpr bool operator==(const Raw& rhs) const noexcept { return m_value == rhs; }
        constexpr bool operator!=(const Raw& rhs) const noexcept { return m_value != rhs; }

       private:
        Raw m_value{Raw::NONE};
    };

    enum CastleFlag : u8 {
        NOCA = 0,
        WKCA = 1,
        WQCA = 2,
        BKCA = 4,
        BQCA = 8
    };

    constexpr PieceType pieceTypeOf(const Piece p) { return static_cast<PieceType>(p & 0x7); }

    constexpr Color pieceColorOf(const Piece p) { return static_cast<Color>(p >> 3); }

    constexpr Piece pieceCreate(const PieceType t, Color c) {
        return static_cast<Piece>((c << 3) | t);
    }

    constexpr Color colorFlip(const Color c) { return static_cast<Color>(c ^ 1); }

    constexpr Rank promotionRankSrcOf(const Color c) {
        return static_cast<Rank>(1 + (5 * (c ^ 1)));
    }
    constexpr Rank promotionRankDestOf(const Color c) { return static_cast<Rank>(7 * (c ^ 1)); }

    const std::string PIECES_STR = "PNBRQKXXpnbrqk.";
    const std::string COLORS_STR = "wb";
    const std::string FILE_STR   = "abcdefgh";
}
