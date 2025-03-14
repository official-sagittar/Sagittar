#include "movepicker.h"

namespace sagittar {

    namespace search {

        void scoreMoves(containers::ArrayList<move::Move>* moves,
                        const board::Board&                board,
                        const move::Move&                  pvmove,
                        const move::Move&                  ttmove,
                        const SearcherData&                data,
                        const i32                          ply) {
            for (u8 i = 0; i < moves->size(); i++)
            {
                const move::Move move = moves->at(i);

                if (move == pvmove)
                {
                    moves->at(i).setScore(PVMOVE_SCORE);
                }
                else if ((ttmove != move::Move()) && (move == ttmove))
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
                    const u32 score = MVV_LVA_TABLE[idx] + MVVLVA_SCORE_OFFSET;
#ifdef DEBUG
                    assert(idx >= 0 && idx < 36);
                    assert(score >= 10100 && score <= 10605);
#endif
                    moves->at(i).setScore(score);
                }
                else
                {
                    if (move == data.killer_moves[0][ply])
                    {
                        moves->at(i).setScore(KILLER_0_SCORE);
                    }
                    else if (move == data.killer_moves[1][ply])
                    {
                        moves->at(i).setScore(KILLER_1_SCORE);
                    }
                    else
                    {
                        const Piece piece = board.getPiece(move.getFrom());
                        const u32   score = std::clamp(data.history[piece][move.getTo()],
                                                       HISTORY_SCORE_MIN, HISTORY_SCORE_MAX);
                        moves->at(i).setScore(score);
                    }
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
