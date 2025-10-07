#pragma once

#include "core/defs.h"

namespace sagittar {

    namespace core {

        enum MoveFlag {
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
           public:
            Move();
            Move(const Square from, const Square to, const MoveFlag flag);
            Move(const Move&)            = default;
            Move& operator=(const Move&) = default;
            bool  operator==(const Move&) const;
            bool  operator!=(const Move&) const;

            uint16_t       id() const;
            Square         from() const;
            Square         to() const;
            MoveFlag       flag() const;
            constexpr bool is_capture() const;
            constexpr bool is_promotion() const;

           private:
            uint16_t m_data;
        };

    }

}
