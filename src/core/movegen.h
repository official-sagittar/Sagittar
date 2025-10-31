#pragma once

#include "commons/containers.h"
#include "commons/pch.h"
#include "core/move.h"
#include "core/position.h"
#include "core/types.h"

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
