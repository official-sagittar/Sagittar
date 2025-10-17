#include "movepicker.h"

namespace sagittar {

    namespace search {

        MovePicker::MovePicker(containers::ArrayList<move::Move>& moves,
                               const board::Board&                board,
                               const move::Move&                  ttmove,
                               const SearcherData&                data,
                               const i32                          ply) :
            m_list(moves),
            m_index(0) {
            scoreMoves(board, ttmove, data, ply);
        }

        void MovePicker::scoreMoves(const board::Board& board,
                                    const move::Move&   ttmove,
                                    const SearcherData& data,
                                    const i32           ply) {
            for (size_t i = 0; i < m_list.size(); i++)
            {
                const move::Move move = m_list.at(i);

                if (move == ttmove)
                {
                    m_list.at(i).setScore(TTMOVE_SCORE);
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
                    m_list.at(i).setScore(score);
                }
                else
                {
                    if (move == data.killer_moves[0][ply])
                    {
                        m_list.at(i).setScore(KILLER_0_SCORE);
                    }
                    else if (move == data.killer_moves[1][ply])
                    {
                        m_list.at(i).setScore(KILLER_1_SCORE);
                    }
                    else
                    {
                        const Piece piece = board.getPiece(move.getFrom());
                        const u32   score = std::clamp(data.history[piece][move.getTo()],
                                                       HISTORY_SCORE_MIN, HISTORY_SCORE_MAX);
                        m_list.at(i).setScore(score);
                    }
                }
            }
        }

        bool MovePicker::has_next() const { return (m_index < m_list.size()); }

        move::Move MovePicker::next() {
            for (size_t i = m_index + 1; i < m_list.size(); i++)
            {
                if (m_list.at(i).getScore() > m_list.at(m_index).getScore())
                {
                    std::swap(m_list.at(m_index), m_list.at(i));
                }
            }

            return m_list.at(m_index++);
        }

    }

}
