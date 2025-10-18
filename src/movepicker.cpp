#include "movepicker.h"

namespace sagittar {

    namespace search {

        MovePicker::MovePicker(const containers::ArrayList<move::Move>& moves,
                               const board::Board&                      board,
                               const move::Move&                        ttmove,
                               const SearcherData&                      data,
                               const i32                                ply) :
            m_moves_count(moves.size()) {
            m_captures.reserve(64);
            m_quiets.reserve(64);
            processMoves(moves, board, ttmove, data, ply);
        }

        void MovePicker::processMoves(const containers::ArrayList<move::Move>& moves,
                                      const board::Board&                      board,
                                      const move::Move&                        ttmove,
                                      const SearcherData&                      data,
                                      const i32                                ply) {
            for (const auto& move : moves)
            {
                if ((move == ttmove) && (ttmove != move::NULL_MOVE))
                {
                    m_tt_move = move;
                }
                else if (move::isCapture(move.getFlag()))
                {
                    const PieceType attacker = pieceTypeOf(board.getPiece(move.getFrom()));
                    const PieceType victim   = (move.getFlag() == move::MoveFlag::MOVE_CAPTURE_EP)
                                               ? PieceType::PAWN
                                               : pieceTypeOf(board.getPiece(move.getTo()));
                    const auto      idx      = mvvlvaIdx(attacker, victim);
                    const auto      score    = MVV_LVA_TABLE[idx];
                    m_captures.emplace_back(move, score);
                }
                else
                {
                    if (move == data.killer_moves[0][ply])
                    {
                        m_killers[0] = move;
                    }
                    else if (move == data.killer_moves[1][ply])
                    {
                        m_killers[1] = move;
                    }
                    else
                    {
                        const Piece piece = board.getPiece(move.getFrom());
                        const auto  score = std::clamp(data.history[piece][move.getTo()],
                                                       HISTORY_SCORE_MIN, HISTORY_SCORE_MAX);
                        m_quiets.emplace_back(move, score);
                    }
                }
            }

            std::ranges::sort(m_captures, std::greater{}, &ScoredMove::score);
            std::ranges::sort(m_quiets, std::greater{}, &ScoredMove::score);
        }

        MovePickerPhase MovePicker::phase() const { return m_phase; }

        bool MovePicker::hasNext() const { return (m_index < m_moves_count); }

        move::Move MovePicker::next() {
            switch (m_phase)
            {
                case MovePickerPhase::TT_MOVE : {
                    m_phase = MovePickerPhase::CAPTURES;
                    if (m_tt_move != move::NULL_MOVE)
                    {
                        ++m_index;
                        return m_tt_move;
                    }
                    [[fallthrough]];
                }

                case MovePickerPhase::CAPTURES : {
                    if (m_index_captures < m_captures.size())
                    {
                        const auto& move = m_captures[m_index_captures++].move;
                        ++m_index;
                        return move;
                    }
                    m_phase = MovePickerPhase::KILLERS;
                    [[fallthrough]];
                }

                case MovePickerPhase::KILLERS : {
                    while (m_index_killers < 2)
                    {
                        const auto& move = m_killers[m_index_killers++];
                        if (move != move::NULL_MOVE)
                        {
                            ++m_index;
                            return move;
                        }
                    }
                    m_phase = MovePickerPhase::QUIETS;
                    [[fallthrough]];
                }

                case MovePickerPhase::QUIETS : {
                    if (m_index_quiets < m_quiets.size())
                    {
                        const auto& move = m_quiets[m_index_quiets++].move;
                        ++m_index;
                        return move;
                    }
                    m_phase = MovePickerPhase::DONE;
                    [[fallthrough]];
                }

                case MovePickerPhase::DONE :
                    [[fallthrough]];

                default :
                    return move::NULL_MOVE;
            }
        }

    }

}
