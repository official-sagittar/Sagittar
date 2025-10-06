#pragma once

#include "core/board.h"
#include "core/defs.h"
#include "core/move.h"
#include "core/position.h"
#include "pch.h"

namespace sagittar {

    namespace core {

        enum class MovegenType {
            MOVEGEN_ALL,
            MOVEGEN_CAPTURES
        };

        void movegen_init();

        BitBoard movegen_get_bishop_attacks(const Square sq, BitBoard occupancy);

        BitBoard movegen_get_rook_attacks(const Square sq, BitBoard occupancy);

        BitBoard
        movegen_get_square_attackers(const Board& board, const Square sq, const Color attacked_by);

        BitBoard movegen_get_pinned_pieces(const Position& pos);

        template<MovegenType T>
        void movegen_generate_pseudolegal_moves(const Position& pos, MoveList* const move_list);

        inline BitBoard ray(const Square sq1, const Square sq2);

        inline BitBoard path_between(const Square sq1, const Square sq2);
    }

}
