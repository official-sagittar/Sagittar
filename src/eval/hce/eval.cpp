#include "eval.h"

namespace sagittar {

    namespace eval {

        namespace hce {

            static constexpr std::array<int, 7> piece_scores = {0, 1, 3, 3, 5, 9, 0};

            void eval_init() {}

            Score eval(const Position* const pos) {

                auto board_ptr = &pos->board;

                const auto w_p = board_ptr->bb_colors[WHITE];
                const auto b_p = board_ptr->bb_colors[BLACK];

                Score score = 0;

                for (int pt = PAWN; pt <= QUEEN; pt++)
                {
                    const auto pt_bb = board_ptr->bb_pieces[pt];
                    score += ((POPCNT(pt_bb & w_p) - POPCNT(pt_bb & b_p)) * piece_scores[pt]);
                }

                const int stm = 1 - (2 * pos->black_to_play);

                score *= stm;

                return score;
            }
        }

    }

}
