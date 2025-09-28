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
        static constexpr std::array<Score, 36> MVV_LVA_SCORE = []() {
            std::array<Score, 36> table = {
                105, 205, 305, 405, 505, 605,
                104, 204, 304, 404, 504, 604,
                103, 203, 303, 403, 503, 603,
                102, 202, 302, 402, 502, 602,
                101, 201, 301, 401, 501, 601,
                100, 200, 300, 400, 500, 600
            };

            const Score offset = 15000;

            for (auto& s : table) {
                s += offset;
            }

            return table;
        }();
        // clang-format on

        static constexpr size_t MVV_LVA_IDX(const PieceType attacker, const PieceType victim) {
            return static_cast<size_t>((attacker - 1) * 6) + (victim - 1);
        }

        static constexpr Score TT_MOVE_SCORE          = 20000;
        static constexpr int   HISTORY_MOVE_SCORE_MAX = 5000;

        static void score_moves(MoveList* const       moves_list,
                                const Position&       pos,
                                const Move            tt_move,
                                const PieceToHistory& history) {
            for (size_t i = 0; i < moves_list->size; i++)
            {
                const Move move = moves_list->moves.at(i);

                if ((move == tt_move) && tt_move)
                {
                    moves_list->scores.at(i) = TT_MOVE_SCORE;
                }
                else if (MOVE_IS_CAPTURE(move))
                {
                    const PieceType attacker = PIECE_TYPE_OF(pos.board.pieces[MOVE_FROM(move)]);
                    const PieceType victim   = (MOVE_FLAG(move) == MOVE_CAPTURE_EP)
                                               ? PAWN
                                               : PIECE_TYPE_OF(pos.board.pieces[MOVE_TO(move)]);
                    assert(attacker != PIECE_TYPE_INVALID);
                    assert(victim != PIECE_TYPE_INVALID);
                    moves_list->scores.at(i) = MVV_LVA_SCORE.at(MVV_LVA_IDX(attacker, victim));
                }
                else
                {
                    const Piece p            = pos.board.pieces[MOVE_FROM(move)];
                    moves_list->scores.at(i) = static_cast<Score>(
                      std::min(history.at(p).at(MOVE_TO(move)), HISTORY_MOVE_SCORE_MAX));
                }
            }
        }

        MovePicker::MovePicker(MoveList* const       moves_list,
                               const Position&       pos,
                               const Move            tt_move,
                               const PieceToHistory& history) :
            index(0),
            list(moves_list) {
            score_moves(moves_list, pos, tt_move, history);
        }

        bool MovePicker::has_next() const { return (index < list->size); }

        std::pair<Move, Score> MovePicker::next() {
            for (size_t i = index + 1; i < list->size; i++)
            {
                if (list->scores.at(i) > list->scores.at(index))
                {
                    std::swap(list->moves.at(i), list->moves.at(index));
                    std::swap(list->scores.at(i), list->scores.at(index));
                }
            }

            const auto val = std::make_pair(list->moves.at(index), list->scores.at(index));

            index++;

            return val;
        }
    }

}
