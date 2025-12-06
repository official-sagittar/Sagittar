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
        Move() noexcept :
            m_data(0) {}
        Move(const Square from, const Square to, const MoveFlag flag) noexcept :
            m_data((flag << 12) | (to << 6) | from) {}
        Move(const u16 data) noexcept :
            m_data(data) {}
        Move(const Move& other) noexcept :
            m_data(other.m_data) {}
        Move(Move&& other) noexcept :
            m_data(other.m_data) {}
        inline Move& operator=(const Move& rhs) noexcept {
            if (this != &rhs)
            {
                m_data = rhs.m_data;
            }
            return *this;
        }
        inline Move& operator=(const Move&& rhs) noexcept {
            if (this != &rhs)
            {
                m_data = rhs.m_data;
            }
            return *this;
        }
        ~Move() noexcept = default;

        inline bool operator==(const Move& rhs) const { return m_data == rhs.m_data; }
        inline bool operator!=(const Move& rhs) const { return m_data != rhs.m_data; }

        inline Square   from() const { return static_cast<Square>(m_data & 0x3F); }
        inline Square   to() const { return static_cast<Square>((m_data >> 6) & 0x3F); }
        inline MoveFlag flag() const { return static_cast<MoveFlag>((m_data >> 12) & 0xF); }
        inline u16      id() const { return m_data; }

        inline bool isCapture() const { return MOVE_IS_CAPTURE(flag()); }
        inline bool isPromotion() const { return MOVE_IS_PROMOTION(flag()); }

        void toString(std::ostringstream&) const;
        void display() const;

       private:
        u16 m_data{0};
    };

    const Move NULL_MOVE = Move{};

    struct ExtMove final {
        Move move{};
        i16  score;

        ExtMove() noexcept :
            move(),
            score(0) {}

        ExtMove(const Move& m, i16 s) noexcept :
            move(m),
            score(s) {}

        ExtMove(const ExtMove& other) noexcept :
            move(other.move),
            score(other.score) {}

        ExtMove(ExtMove&& other) noexcept :
            move(std::move(other.move)),
            score(other.score) {}

        ExtMove& operator=(const ExtMove& other) noexcept {
            if (this != &other)
            {
                move  = other.move;
                score = other.score;
            }
            return *this;
        }

        ExtMove& operator=(ExtMove&& other) noexcept {
            if (this != &other)
            {
                move  = std::move(other.move);
                score = other.score;
            }
            return *this;
        }

        ~ExtMove() noexcept = default;
    };
}
