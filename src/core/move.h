#pragma once

#include "core/defs.h"
#include "pch.h"

namespace sagittar {

    namespace core {

        constexpr int MOVE_LIST_SIZE_MAX = 256;

        using Move = uint16_t;

        constexpr Move NULL_MOVE = 0;

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

        inline constexpr Move MOVE_CREATE(const Square from, const Square to, const MoveFlag flag) {
            return static_cast<Move>((flag << 12) | (to << 6) | from);
        }
        inline constexpr Square MOVE_FROM(const Move m) { return static_cast<Square>(m & 0x3F); }
        inline constexpr Square MOVE_TO(const Move m) {
            return static_cast<Square>((m >> 6) & 0x3F);
        }
        inline constexpr MoveFlag MOVE_FLAG(const Move m) {
            return static_cast<MoveFlag>((m >> 12) & 0xF);
        }
        inline constexpr PieceType MOVE_PROMOTED_PIECE_TYPE(const Move m) {
            return static_cast<PieceType>(((MOVE_FLAG(m)) & 3) + 2);
        }
        inline constexpr bool MOVE_IS_CAPTURE(const Move m) {
            return static_cast<bool>(MOVE_FLAG(m) & 0x4);
        }
        inline constexpr bool MOVE_IS_PROMOTION(const Move m) {
            return static_cast<bool>(MOVE_FLAG(m) & 0x8);
        }
        inline constexpr bool MOVE_IS_CAPTURE(const MoveFlag f) {
            return static_cast<bool>(f & 0x4);
        }
        inline constexpr bool MOVE_IS_PROMOTION(const MoveFlag f) {
            return static_cast<bool>(f & 0x8);
        }

        std::string move_tostring(const Move m);

        class MoveList {
           public:
            class Iterator {
               public:
                using iterator_category = std::forward_iterator_tag;
                using value_type        = std::pair<Move, Score>;
                using difference_type   = std::ptrdiff_t;
                using pointer           = value_type*;
                using reference         = value_type;

                Iterator(Move* m, Score* s) :
                    move_ptr(m),
                    score_ptr(s) {}

                value_type operator*() const { return {*move_ptr, *score_ptr}; }

                Iterator& operator++() {
                    ++move_ptr;
                    ++score_ptr;
                    return *this;
                }

                Iterator operator++(int) {
                    Iterator tmp = *this;
                    ++(*this);
                    return tmp;
                }

                bool operator==(const Iterator& other) const {
                    return move_ptr == other.move_ptr && score_ptr == other.score_ptr;
                }

                bool operator!=(const Iterator& other) const { return !(*this == other); }

               private:
                Move*  move_ptr;
                Score* score_ptr;
            };

            MoveList();
            void     add_move(const Move move);
            Iterator begin() { return Iterator(moves.data(), scores.data()); }
            Iterator end() { return Iterator(moves.data() + size, scores.data() + size); }
            ~MoveList() = default;

           private:
            std::array<Move, MOVE_LIST_SIZE_MAX>  moves;
            std::array<Score, MOVE_LIST_SIZE_MAX> scores;
            size_t                                size;
        };

    }
}
