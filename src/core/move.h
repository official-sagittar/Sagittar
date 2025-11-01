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

    class Move final {
       public:
        Move();
        Move(const Move& other);
        Move(Move&& other);
        Move& operator=(const Move& rhs);
        Move& operator=(const Move&& rhs);
        ~Move() = default;

        Move(const Square from, const Square to, const MoveFlag flag);
        Move(const u16 data);

        bool operator==(const Move& rhs) const;
        bool operator!=(const Move& rhs) const;

        static Move fromId(const u16 id);

        Square   from() const;
        Square   to() const;
        MoveFlag flag() const;
        u16      id() const;

        bool isCapture() const;
        bool isPromotion() const;

        void toString(std::ostringstream&) const;
        void display() const;

       private:
        u16 m_data{0};
    };

    const Move NULL_MOVE = Move{};

    struct ExtMove final {
        Move move{};
        u32  score;

        ExtMove() noexcept :
            move(),
            score(0) {}

        ExtMove(const Move& m, u32 s) noexcept :
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
