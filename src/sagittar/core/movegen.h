#pragma once

#include "commons/containers.h"
#include "commons/pch.h"
#include "sagittar/core/board.h"
#include "sagittar/core/move.h"
#include "sagittar/core/types.h"

namespace sagittar {

    namespace core {

        namespace movegen {

            using namespace core::types;

            enum class MovegenType {
                ALL,
                CAPTURES
            };

            void initialize();

            bool
            isSquareAttacked(const board::Board& board, const Square sq, const Color attacked_by);

            bool isInCheck(const board::Board& board);

            void generatePseudolegalMoves(commons::containers::ArrayList<move::Move>* moves,
                                          const board::Board&                         board,
                                          const MovegenType                           type);

        }  // namespace movegen

    }  // namespace core

}  // namespace sagittar
