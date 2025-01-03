#pragma once

#include "pch.h"
#include "types.h"

namespace sagittar {

    namespace move {

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

        class Move {
           private:
            Square   from;
            Square   to;
            MoveFlag flag;
            u16      score;

           public:
            Move();
            Move(const Move&);
            Move(const Square from, const Square to, const MoveFlag flag);
            void     setScore(const u16);
            Square   getFrom() const;
            Square   getTo() const;
            MoveFlag getFlag() const;
            u16      getScore() const;
            u32      id() const;
            void     toString(std::stringstream&) const;
            void     display() const;
            Move&    operator=(Move const&);
            bool     operator==(Move const& rhs);
        };

        constexpr bool isCapture(const MoveFlag m) { return (m & 0x4); }

        constexpr bool isPromotion(const MoveFlag m) { return (m & 0x8); }

    }

}
