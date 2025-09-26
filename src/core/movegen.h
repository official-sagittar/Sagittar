#pragma once

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

        BitBoard
        movegen_get_square_attackers(const Position& pos, const Square sq, const Color attacked_by);

        template<MovegenType T>
        void movegen_generate_pseudolegal_moves(const Position& pos, MoveList* const move_list);
    }

}
