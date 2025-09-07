#include "eval.h"
#include "core/utils.h"

namespace sagittar {

    namespace eval {

        namespace hce {

            using PSQT = std::array<Score, 64>;

            static constexpr std::array<Score, 7> piece_scores = {0, 100, 320, 330, 500, 900, 0};

            static constexpr std::array<PSQT, 7> psqt_b = []() {
                std::array<PSQT, 7> table = {};

                // clang-format off
                table[PIECE_TYPE_INVALID] = std::array<Score, 64>{};

                table[PAWN] = std::array<Score, 64> {
                      0,   0,   0,   0,   0,   0,   0,   0,
                     50,  50,  50,  50,  50,  50,  50,  50,
                     10,  10,  20,  30,  30,  20,  10,  10,
                      5,   5,  10,  25,  25,  10,   5,   5,
                      0,   0,   0,  20,  20,   0,   0,   0,
                      5,  -5, -10,   0,   0, -10,  -5,   5,
                      5,  10,  10, -20, -20,  10,  10,   5,
                      0,   0,   0,   0,   0,   0,   0,   0
                };

                table[KNIGHT] = std::array<Score, 64> {
                    -50, -40, -30, -30, -30, -30, -40, -50,
                    -40, -20,   0,   0,   0,   0, -20, -40,
                    -30,   0,  10,  15,  15,  10,   0, -30,
                    -30,   5,  15,  20,  20,  15,   5, -30,
                    -30,   0,  15,  20,  20,  15,   0, -30,
                    -30,   5,  10,  15,  15,  10,   5, -30,
                    -40, -20,   0,   5,   5,   0, -20, -40,
                    -50, -40, -30, -30, -30, -30, -40, -50
                };

                table[BISHOP] = std::array<Score, 64> {
                    -20, -10, -10, -10, -10, -10, -10, -20,
                    -10,   0,   0,   0,   0,   0,   0, -10,
                    -10,   0,   5,  10,  10,   5,   0, -10,
                    -10,   5,   5,  10,  10,   5,   5, -10,
                    -10,   0,  10,  10,  10,  10,   0, -10,
                    -10,  10,  10,  10,  10,  10,  10, -10,
                    -10,   5,   0,   0,   0,   0,   5, -10,
                    -20, -10, -10, -10, -10, -10, -10, -20
                };

                table[ROOK] = std::array<Score, 64> {
                      0,   0,   0,   0,   0,   0,   0,   0,
                      5,  10,  10,  10,  10,  10,  10,   5,
                     -5,   0,   0,   0,   0,   0,   0,  -5,
                     -5,   0,   0,   0,   0,   0,   0,  -5,
                     -5,   0,   0,   0,   0,   0,   0,  -5,
                     -5,   0,   0,   0,   0,   0,   0,  -5,
                     -5,   0,   0,   0,   0,   0,   0,  -5,
                      0,   0,   0,   5,   5,   0,   0,   0
                };
                table[QUEEN] = std::array<Score, 64> {
                    -20, -10, -10, -5, -5, -10, -10, -20,
                    -10,   0,   0,  0,  0,   0,   0, -10,
                    -10,   0,   5,  5,  5,   5,   0, -10,
                     -5,   0,   5,  5,  5,   5,   0,  -5,
                      0,   0,   5,  5,  5,   5,   0,  -5,
                    -10,   5,   5,  5,  5,   5,   0, -10,
                    -10,   0,   5,  0,  0,   0,   0, -10,
                    -20, -10, -10, -5, -5, -10, -10, -20
                };

                table[KING] = std::array<Score, 64> {
                    -30, -40, -40, -50, -50, -40, -40, -30,
                    -30, -40, -40, -50, -50, -40, -40, -30,
                    -30, -40, -40, -50, -50, -40, -40, -30,
                    -30, -40, -40, -50, -50, -40, -40, -30,
                    -20, -30, -30, -40, -40, -30, -30, -20,
                    -10, -20, -20, -20, -20, -20, -20, -10,
                     20,  20,   0,   0,   0,   0,  20,  20,
                     20,  30,  10,   0,   0,  10,  30,  20
                };
                // clang-format on

                for (int pt = PIECE_TYPE_INVALID; pt <= KING; pt++)
                {
                    for (int sq = A1; sq <= H8; sq++)
                    {
                        table[pt][sq] += piece_scores[pt];
                    }
                }

                return table;
            }();

            static constexpr std::array<PSQT, 7> psqt_w = []() {
                std::array<PSQT, 7> table = {};

                for (int sq = A1; sq <= H8; sq++)
                {
                    const int mirrored_sq = sq ^ 56;
                    for (int pt = PIECE_TYPE_INVALID; pt <= KING; pt++)
                    {
                        table[pt][sq] = psqt_b[pt][mirrored_sq];
                    }
                }

                return table;
            }();

            void eval_init() {}

            Score eval(const Position* const pos) {

                auto board_ptr = &pos->board;

                const auto w_p = board_ptr->bb_colors[WHITE];
                const auto b_p = board_ptr->bb_colors[BLACK];

                Score score = 0;

                for (int pt = PAWN; pt <= KING; pt++)
                {
                    const auto pt_bb = board_ptr->bb_pieces[pt];
                    // WHITE
                    auto pt_bb_w = pt_bb & w_p;
                    while (pt_bb_w)
                    {
                        const auto sq = bitscan_forward(&pt_bb_w);
                        score += psqt_w[pt][sq];
                    }
                    // BLACK
                    auto pt_bb_b = pt_bb & b_p;
                    while (pt_bb_b)
                    {
                        const auto sq = bitscan_forward(&pt_bb_b);
                        score -= psqt_b[pt][sq];
                    }
                }

                const int stm = 1 - (2 * pos->black_to_play);

                score *= stm;

                return score;
            }
        }

    }

}
