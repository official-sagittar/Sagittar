#pragma once

#include "core/board.h"
#include "core/defs.h"
#include "core/move.h"
#include "pch.h"

namespace sagittar {

    namespace core {

        class PositionHistory {
           public:
            PositionHistory();
            void     reset();
            void     push(const uint64_t hash);
            uint64_t peek(const size_t i);
            void     pop();
            ~PositionHistory() = default;

           private:
            static constexpr int HISTORY_SIZE_MAX = 2048;

            std::vector<uint64_t> hash_history;
        };

        class Position {
           public:
            Position();
            Position(const Position&) = default;
            void      reset();
            void      reset_hash();
            bool      set_fen(std::string);
            bool      is_valid() const;
            bool      is_repeated(PositionHistory* const history) const;
            bool      is_in_check() const;
            bool      do_move(const Move move, PositionHistory* const history);
            bool      do_move(const std::string& move_str, PositionHistory* const history);
            void      undo_move(PositionHistory* const history);
            void      display() const;
            Position& operator=(const Position&) = default;
            ~Position()                          = default;

           public:
            Board    board;
            bool     black_to_play;
            uint8_t  ca_rights;
            Square   ep_target;
            uint32_t half_moves, full_moves, ply_count;
            // Hash
            uint64_t hash;
            // Status
            Square   king_sq;
            BitBoard checkers;
        };

        void position_init();

    }

}
