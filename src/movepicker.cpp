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
        const u32 MVV_LVA_TABLE[36] = {
            105, 205, 305, 405, 505, 605,
            104, 204, 304, 404, 504, 604,
            103, 203, 303, 403, 503, 603,
            102, 202, 302, 402, 502, 602,
            101, 201, 301, 401, 501, 601,
            100, 200, 300, 400, 500, 600
        };
        // clang-format on

        constexpr int mvvlvaIdx(const PieceType attacker, const PieceType victim) {
            return ((attacker - 1) * 6) + (victim - 1);
        }

        constexpr u32 MVVLVA_SCORE_OFFSET = 10000;
        constexpr u32 HISTORY_SCORE_MIN   = 0;
        constexpr u32 HISTORY_SCORE_MAX   = 7000;

        template<movegen::MovegenType T>
        MovePicker<T>::MovePicker(move::ExtMove*      buffer,
                                  const board::Board& board,
                                  const move::Move&   ttmove,
                                  const SearcherData& data,
                                  const i32           ply) {
            process(buffer, board, ttmove, data, ply);
        }

        template<movegen::MovegenType T>
        void MovePicker<T>::process(move::ExtMove*      buffer,
                                    const board::Board& board,
                                    const move::Move&   ttmove,
                                    const SearcherData& data,
                                    const i32           ply) {
            // Generate pseudolegal moves
            containers::ArrayList<move::Move> moves;
            movegen::generatePseudolegalMoves<T>(&moves, board);

            // Save number of moves
            m_moves_count = moves.size();

            // Process TT Move and Captures

            size_t capture_count = 0;

            for (const auto& move : moves)
            {
                if ((move == ttmove) && (ttmove != move::NULL_MOVE))
                {
                    m_tt_move = move;
                    continue;
                }

                if (move::isCapture(move.getFlag()))
                {
                    const PieceType attacker = pieceTypeOf(board.getPiece(move.getFrom()));
                    const PieceType victim   = (move.getFlag() == move::MoveFlag::MOVE_CAPTURE_EP)
                                               ? PieceType::PAWN
                                               : pieceTypeOf(board.getPiece(move.getTo()));
                    const auto      idx      = mvvlvaIdx(attacker, victim);
                    const auto      score    = MVV_LVA_TABLE[idx] + MVVLVA_SCORE_OFFSET;
                    buffer[capture_count++]  = move::ExtMove{move, score};
                }
            }

            // Process Killer moves and Quites

            move::ExtMove* quiet_ptr = buffer + capture_count;

            for (const auto& move : moves)
            {
                if (move::isCapture(move.getFlag()) || (move == ttmove))
                {
                    continue;
                }

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
                    *quiet_ptr++      = move::ExtMove{move, score};
                }
            }

            const size_t quiet_count = quiet_ptr - (buffer + capture_count);

            m_captures = std::span<move::ExtMove>(buffer, capture_count);
            m_quiets   = std::span<move::ExtMove>(buffer + capture_count, quiet_count);

            std::ranges::sort(m_captures, std::greater{}, &move::ExtMove::score);
            std::ranges::sort(m_quiets, std::greater{}, &move::ExtMove::score);

            m_it_caps   = m_captures.data();
            m_it_quiets = m_quiets.data();
        }

        template<movegen::MovegenType T>
        size_t MovePicker<T>::size() const {
            return m_moves_count;
        }

        template<movegen::MovegenType T>
        MovePickerPhase MovePicker<T>::phase() const {
            return m_phase;
        }

        template<movegen::MovegenType T>
        bool MovePicker<T>::hasNext() const {
            return (m_index < m_moves_count);
        }

        template<movegen::MovegenType T>
        move::Move MovePicker<T>::next() {
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
                    if (m_it_caps != (m_captures.data() + m_captures.size()))
                    {
                        ++m_index;
                        return (m_it_caps++)->move;
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
                    if (m_it_quiets != (m_quiets.data() + m_quiets.size()))
                    {
                        ++m_index;
                        return (m_it_quiets++)->move;
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

        template class MovePicker<movegen::MovegenType::ALL>;
        template class MovePicker<movegen::MovegenType::CAPTURES>;

    }

}
