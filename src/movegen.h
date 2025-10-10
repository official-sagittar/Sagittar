#pragma once

#include "board.h"
#include "containers.h"
#include "move.h"
#include "pch.h"
#include "types.h"

namespace sagittar {

    namespace movegen {

        enum class MovegenType {
            ALL,
            CAPTURES
        };

        void initialize();

        board::BitBoard
        getSquareAttackers(const board::Board& board, const Square sq, const Color attacked_by);

        template<MovegenType T>
        void generatePseudolegalMoves(containers::ArrayList<move::Move>* moves,
                                      const board::Board&                board);

    }

}
