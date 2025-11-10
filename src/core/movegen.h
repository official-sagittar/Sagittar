#pragma once

#include "commons/containers.h"
#include "commons/pch.h"
#include "core/bitboard.h"
#include "core/move.h"
#include "core/position.h"
#include "core/types.h"

namespace sagittar {

    enum class MovegenType {
        ALL,
        CAPTURES
    };

    void movegen_initialize();

    template<PieceType PT>
    BitBoard attacks(const Square sq, const BitBoard occupancy, const Color c = Color::WHITE);

    BitBoard squareAttackers(const Position& pos, const Square sq, const Color attacked_by);

    template<MovegenType T>
    void pseudolegalMoves(containers::ArrayList<Move>* moves, const Position& pos);

}
