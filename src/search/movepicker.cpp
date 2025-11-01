#include "movepicker.h"

namespace sagittar::search {

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
    static const u32 MVV_LVA_TABLE[36] = {
        105, 205, 305, 405, 505, 605,
        104, 204, 304, 404, 504, 604,
        103, 203, 303, 403, 503, 603,
        102, 202, 302, 402, 502, 602,
        101, 201, 301, 401, 501, 601,
        100, 200, 300, 400, 500, 600
    };
    // clang-format on

    static constexpr int mvvlvaIdx(const PieceType attacker, const PieceType victim) {
        return ((attacker - 1) * 6) + (victim - 1);
    }

    static const auto cmp = [](const ExtMove& a, const ExtMove& b) {
        if (a.score != b.score)
            return a.score > b.score;
        return a.move.id() < b.move.id();
    };

    constexpr u32 MVVLVA_SCORE_OFFSET = 10000;
    constexpr u32 HISTORY_SCORE_MIN   = 0;
    constexpr u32 HISTORY_SCORE_MAX   = 7000;

    template<MovegenType T>
    MovePicker<T>::MovePicker(ExtMove*            buffer,
                              const Position&     pos,
                              const Move&         ttmove,
                              const SearcherData& data,
                              const i32           ply) {
        process(buffer, pos, ttmove, data, ply);
    }

    template<MovegenType T>
    void MovePicker<T>::process(ExtMove*            buffer,
                                const Position&     pos,
                                const Move&         ttmove,
                                const SearcherData& data,
                                const i32           ply) {
        // Generate pseudolegal moves
        containers::ArrayList<Move> moves;
        generatePseudolegalMoves<T>(&moves, pos);

        // Save number of moves
        m_moves_count = moves.size();

        // Process TT Move and Captures

        size_t capture_count = 0;

        for (const auto& move : moves)
        {
            if ((move == ttmove) && (ttmove != NULL_MOVE))
            {
                m_tt_move = move;
                continue;
            }

            if (move.isCapture())
            {
                const PieceType attacker = pieceTypeOf(pos.pieceOn(move.from()));
                const PieceType victim   = (move.flag() == MoveFlag::MOVE_CAPTURE_EP)
                                           ? PieceType::PAWN
                                           : pieceTypeOf(pos.pieceOn(move.to()));
                const auto      idx      = mvvlvaIdx(attacker, victim);
                const auto      score    = MVV_LVA_TABLE[idx] + MVVLVA_SCORE_OFFSET;
                buffer[capture_count++]  = ExtMove{move, score};
            }
        }

        // Process Killer moves and Quites

        ExtMove* quiet_ptr = buffer + capture_count;

        for (const auto& move : moves)
        {
            if (move.isCapture() || (move == ttmove))
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
                const Piece piece = pos.pieceOn(move.from());
                const auto  score =
                  std::clamp(data.history[piece][move.to()], HISTORY_SCORE_MIN, HISTORY_SCORE_MAX);
                *quiet_ptr++ = ExtMove{move, score};
            }
        }

        const size_t quiet_count = quiet_ptr - (buffer + capture_count);

        m_captures = std::span<ExtMove>(buffer, capture_count);
        m_quiets   = std::span<ExtMove>(buffer + capture_count, quiet_count);

        std::ranges::sort(m_captures, cmp);
        std::ranges::sort(m_quiets, cmp);

        m_it_caps   = m_captures.data();
        m_it_quiets = m_quiets.data();
    }

    template<MovegenType T>
    size_t MovePicker<T>::size() const {
        return m_moves_count;
    }

    template<MovegenType T>
    MovePickerPhase MovePicker<T>::phase() const {
        return m_phase;
    }

    template<MovegenType T>
    bool MovePicker<T>::hasNext() const {
        return (m_index < m_moves_count);
    }

    template<MovegenType T>
    Move MovePicker<T>::next() {
        switch (m_phase)
        {
            case MovePickerPhase::TT_MOVE : {
                m_phase = MovePickerPhase::CAPTURES;
                if (m_tt_move != NULL_MOVE)
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
                    if (move != NULL_MOVE)
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
                return NULL_MOVE;
        }
    }

    template class MovePicker<MovegenType::ALL>;
    template class MovePicker<MovegenType::CAPTURES>;

}
