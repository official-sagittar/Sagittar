#include "core/position.h"
#include "core/types.h"
#include "doctest/doctest.h"
#include "eval/hce/defs.h"

using namespace sagittar;

TEST_SUITE("Eval") {

    TEST_CASE("eval::phase") {
        Position pos;

        pos.setFen("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");
        CHECK(eval::hce::pos_phase(pos) == 0);

        pos.setFen("4k3/8/8/8/8/8/8/4K3 w - - 0 1");
        CHECK(eval::hce::pos_phase(pos) == 256);
    }

    TEST_CASE("eval::std_phase") {
        const auto raw_phase = [](const Position pos) -> i32 {
            i32 phase = eval::hce::TOTAL_PHASE;

            const auto w_p = pos.pieces(Color::WHITE);
            const auto b_p = pos.pieces(Color::BLACK);

            for (int pt = PieceType::PAWN; pt <= PieceType::KING; pt++)
            {
                const auto pt_bb = pos.pieces(static_cast<PieceType>(pt));
                // WHITE
                auto pt_bb_w = pt_bb & w_p;
                while (pt_bb_w)
                {
                    phase -= eval::hce::PHASE_WEIGHTS[pt];
                    (void) pt_bb_w.pop_lsb();
                }
                // BLACK
                auto pt_bb_b = pt_bb & b_p;
                while (pt_bb_b)
                {
                    phase -= eval::hce::PHASE_WEIGHTS[pt];
                    (void) pt_bb_b.pop_lsb();
                }
            }

            return phase;
        };

        Position pos;

        // Startpos
        pos.setFen("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");

        i32 phase = raw_phase(pos);
        phase     = eval::hce::std_phase(phase);

        CHECK(phase == 0);
        CHECK(phase == eval::hce::pos_phase(pos));

        // End Game
        pos.setFen("4k3/8/8/8/8/8/8/4K3 w - - 0 1");

        phase = raw_phase(pos);
        phase = eval::hce::std_phase(phase);

        CHECK(phase == 256);
        CHECK(phase == eval::hce::pos_phase(pos));
    }

    TEST_CASE("eval::scale_eval") {
        CHECK(eval::hce::scale_eval(100, 200, 0) == 100);
        CHECK(eval::hce::scale_eval(100, 200, 256) == 200);
        CHECK(eval::hce::scale_eval(100, 200, 128) == 150);
    }

    TEST_CASE("eval::phase_factors") {
        Position pos;

        // Startpos
        pos.setFen("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");

        i32    phase = eval::hce::pos_phase(pos);
        double w_eg  = phase / 256.0;
        double w_mg  = 1.0 - w_eg;

        CHECK(w_mg == 1.0);
        CHECK(w_eg == 0.0);

        // Eng Game
        pos.setFen("4k3/8/8/8/8/8/8/4K3 w - - 0 1");

        phase = eval::hce::pos_phase(pos);
        w_eg  = phase / 256.0;
        w_mg  = 1.0 - w_eg;

        CHECK(w_mg == 0.0);
        CHECK(w_eg == 1.0);
    }

    TEST_CASE("eval::mg_eg_score") {
        i32 score = eval::hce::S(100, 200);

        CHECK(eval::hce::mg_score(score) == 100);
        CHECK(eval::hce::eg_score(score) == 200);
    }
}
