#pragma once

#include "core/board.h"
#include "core/defs.h"
#include "core/move.h"
#include "pch.h"

namespace sagittar {

    namespace core {

        class Position {
           public:
            Position();
            Position(const Position&) = default;
            void      reset();
            void      reset_hash();
            bool      set_fen(std::string);
            bool      is_valid() const;
            bool      is_repeated() const;
            bool      is_in_check() const;
            bool      do_move(const Move move);
            bool      do_move(const std::string& move_str);
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
