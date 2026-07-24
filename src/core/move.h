#pragma once

#include "commons/pch.h"
#include "core/types.h"

namespace sagittar {

    enum MoveFlag : u8 {
        MOVE_QUIET,
        MOVE_QUIET_PAWN_DBL_PUSH,
        MOVE_CASTLE_KING_SIDE,
        MOVE_CASTLE_QUEEN_SIDE,
        MOVE_CAPTURE,
        MOVE_CAPTURE_EP,
        MOVE_PROMOTION_KNIGHT = 8,
        MOVE_PROMOTION_BISHOP,
        MOVE_PROMOTION_ROOK,
        MOVE_PROMOTION_QUEEN,
        MOVE_CAPTURE_PROMOTION_KNIGHT,
        MOVE_CAPTURE_PROMOTION_BISHOP,
        MOVE_CAPTURE_PROMOTION_ROOK,
        MOVE_CAPTURE_PROMOTION_QUEEN,
    };

    inline constexpr bool MOVE_IS_CAPTURE(const MoveFlag f) { return static_cast<bool>(f & 0x4); }
    inline constexpr bool MOVE_IS_PROMOTION(const MoveFlag f) { return static_cast<bool>(f & 0x8); }

    class Move final {
       public:
        constexpr Move() noexcept = default;
        explicit constexpr Move(const Square from, const Square to, const MoveFlag flag) noexcept :
            m_data(static_cast<u16>((flag << 12) | (to << 6) | from)) {}
        explicit constexpr Move(const u16 data) noexcept :
            m_data(data) {}

        constexpr bool operator==(const Move& rhs) const noexcept { return m_data == rhs.m_data; }
        constexpr bool operator!=(const Move& rhs) const noexcept { return m_data != rhs.m_data; }

        [[nodiscard]] constexpr Square from() const noexcept {
            return static_cast<Square>(m_data & 0x3F);
        }
        [[nodiscard]] constexpr Square to() const noexcept {
            return static_cast<Square>((m_data >> 6) & 0x3F);
        }
        [[nodiscard]] constexpr MoveFlag flag() const noexcept {
            return static_cast<MoveFlag>((m_data >> 12) & 0xF);
        }
        [[nodiscard]] constexpr u16 id() const noexcept { return m_data; }

        [[nodiscard]] constexpr bool isCapture() const noexcept { return MOVE_IS_CAPTURE(flag()); }
        [[nodiscard]] constexpr bool isPromotion() const noexcept {
            return MOVE_IS_PROMOTION(flag());
        }

        void toString(std::ostringstream&) const;
        void display() const;

       private:
        u16 m_data{0};
    };

    static_assert(sizeof(Move) == 2);
    static_assert(std::is_trivially_copyable_v<Move>);
    static_assert(std::is_trivially_copy_constructible_v<Move>);
    static_assert(std::is_trivially_copy_assignable_v<Move>);
    static_assert(std::is_trivially_destructible_v<Move>);

    const Move NULL_MOVE = Move{};

    struct ExtMove final {
        Move move{};
        i16  score{0};

        constexpr ExtMove() noexcept = default;

        explicit constexpr ExtMove(const Move& m, i16 s) noexcept :
            move(m),
            score(s) {}
    };

    static_assert(sizeof(ExtMove) == 4);
    static_assert(std::is_trivially_copyable_v<ExtMove>);
    static_assert(std::is_trivially_copy_constructible_v<ExtMove>);
    static_assert(std::is_trivially_copy_assignable_v<ExtMove>);
    static_assert(std::is_trivially_destructible_v<ExtMove>);
}
