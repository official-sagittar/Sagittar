#pragma once

#include "board.h"
#include "containers.h"
#include "move.h"
#include "pch.h"
#include "tt.h"
#include "types.h"

namespace sagittar {

    namespace search {

        void scoreMoves(containers::ArrayList<move::Move>* moves,
                        const board::Board&                board,
                        const move::Move&                  pvmove,
                        const tt::TranspositionTable&      ttable);
        void sortMoves(containers::ArrayList<move::Move>* moves, const u8 index);

    }

}
