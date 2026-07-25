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
        CAPTURES,
        CHECK_EVASIONS
    };

    void movegen_initialize();

    template<PieceType PT>
    BitBoard attacks(const Square sq, const BitBoard occupancy, const Color c = Color::WHITE);

    [[nodiscard]] inline BitBoard attacks(const PieceType pt,
                                          const Square    sq,
                                          const BitBoard  occupancy,
                                          const Color     c = Color::WHITE) noexcept {
        switch (pt)
        {
            case PieceType::PAWN :
                return attacks<PieceType::PAWN>(sq, occupancy, c);
            case PieceType::KNIGHT :
                return attacks<PieceType::KNIGHT>(sq, occupancy, c);
            case PieceType::BISHOP :
                return attacks<PieceType::BISHOP>(sq, occupancy, c);
            case PieceType::ROOK :
                return attacks<PieceType::ROOK>(sq, occupancy, c);
            case PieceType::QUEEN :
                return attacks<PieceType::QUEEN>(sq, occupancy, c);
            case PieceType::KING :
                return attacks<PieceType::KING>(sq, occupancy, c);
            default :
                assert(false);
                break;
        }

        return 0ULL;
    }

    BitBoard squareAttackers(const Position& pos, const Square sq, const Color attacked_by);

    template<MovegenType T>
    void pseudolegalMoves(containers::ArrayList<Move>* moves, const Position& pos);

}
