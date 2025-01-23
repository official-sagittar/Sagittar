#include "movepicker.h"

namespace sagittar {

    namespace search {

        // clang-format off
        /*
            (Victims) Pawn   Knight Bishop Rook   Queen  King
        (Attackers)
        Pawn          105    205    305    405    505    605
        Knight        104    204    304    404    504    604
        Bishop        103    203    303    403    503    603
        Rook          102    202    302    402    502    602
        Queen         101    201    301    401    501    601
        King          100    200    300    400    500    600
        */
        static const u16 MVV_LVA_TABLE[36] = {
            105, 205, 305, 405, 505, 605,
            104, 204, 304, 404, 504, 604,
            103, 203, 303, 403, 503, 603,
            102, 202, 302, 402, 502, 602,
            101, 201, 301, 401, 501, 601,
            100, 200, 300, 400, 500, 600
        };
        // clang-format on

        static constexpr u16 PVMOVE_SCORE        = 40000;
        static constexpr u16 TTMOVE_SCORE        = 30000;
        static constexpr u16 MVVLVA_SCORE_OFFSET = 10000;

        static constexpr u8 mvvlvaIdx(const PieceType attacker, const PieceType victim) {
            return ((attacker - 1) * 6) + (victim - 1);
        }

        void scoreMoves(containers::ArrayList<move::Move>* moves,
                        const board::Board&                board,
                        const move::Move&                  pvmove,
                        const tt::TranspositionTable&      ttable) {
            move::Move  ttmove;
            bool        ttmove_found = false;
            tt::TTEntry ttentry;
            const bool  tthit = ttable.probe(&ttentry, board);
            if (tthit)
            {
                ttmove       = ttentry.move;
                ttmove_found = true;
            }

            for (u8 i = 0; i < moves->size(); i++)
            {
                const move::Move move = moves->at(i);

                if (move == pvmove)
                {
                    moves->at(i).setScore(PVMOVE_SCORE);
                }
                else if (ttmove_found && (move == ttmove))
                {
                    moves->at(i).setScore(TTMOVE_SCORE);
                }
                else if (move::isCapture(move.getFlag()))
                {
                    const PieceType attacker = pieceTypeOf(board.getPiece(move.getFrom()));
                    const PieceType victim   = (move.getFlag() == move::MoveFlag::MOVE_CAPTURE_EP)
                                               ? PieceType::PAWN
                                               : pieceTypeOf(board.getPiece(move.getTo()));
#ifdef DEBUG
                    assert(attacker != 0);
                    assert(victim != 0);
#endif
                    const u8  idx   = mvvlvaIdx(attacker, victim);
                    const u16 score = MVV_LVA_TABLE[idx] + MVVLVA_SCORE_OFFSET;
#ifdef DEBUG
                    assert(idx >= 0 && idx < 36);
                    assert(score >= 10100 && score <= 10605);
#endif
                    moves->at(i).setScore(score);
                }
            }
        }

        void sortMoves(containers::ArrayList<move::Move>* moves, const u8 index) {
            for (u32 i = index + 1; i < moves->size(); i++)
            {
                if (moves->at(i).getScore() > moves->at(index).getScore())
                {
                    std::swap(moves->at(index), moves->at(i));
                }
            }
        }

    }

}
