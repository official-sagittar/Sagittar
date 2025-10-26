#pragma once

#include "containers.h"
#include "move.h"
#include "pch.h"
#include "position.h"
#include "types.h"

namespace sagittar {

    namespace movegen {

        enum class MovegenType {
            ALL,
            CAPTURES
        };

        void initialize();

        core::BitBoard
        getSquareAttackers(const core::Position& pos, const Square sq, const Color attacked_by);

        template<MovegenType T>
        void generatePseudolegalMoves(containers::ArrayList<move::Move>* moves,
                                      const core::Position&              pos);

    }

}
