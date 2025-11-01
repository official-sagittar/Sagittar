#pragma once

#include "commons/containers.h"
#include "commons/pch.h"
#include "core/move.h"
#include "core/position.h"
#include "core/types.h"

namespace sagittar {

    enum class MovegenType {
        ALL,
        CAPTURES
    };

    void movegen_initialize();

    BitBoard getSquareAttackers(const Position& pos, const Square sq, const Color attacked_by);

    template<MovegenType T>
    void generatePseudolegalMoves(containers::ArrayList<Move>* moves, const Position& pos);

}
