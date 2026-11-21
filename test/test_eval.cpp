#include "commons/pch.h"
#include "commons/utils.h"
#include "core/position.h"
#include "core/types.h"
#include "doctest/doctest.h"
#include "eval/hce/defs.h"

using namespace sagittar;

TEST_SUITE("Eval") {

    TEST_CASE("eval::phase") {
        Position pos;

        pos.setFen("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");
        CHECK(eval::hce::posPhase(pos) == 0);

        pos.setFen("4k3/8/8/8/8/8/8/4K3 w - - 0 1");
        CHECK(eval::hce::posPhase(pos) == 256);
    }

    TEST_CASE("eval::stdPhase") {
        const auto raw_phase = [](const Position pos) -> I32 {
            I32 phase = eval::hce::k_TOTAL_PHASE;

            const auto w_p = pos.pieces(Color::WHITE);
            const auto b_p = pos.pieces(Color::BLACK);

            for (int pt = PieceType::PAWN; pt <= PieceType::KING; pt++)
            {
                const auto pt_bb = pos.pieces(static_cast<PieceType>(pt));
                // WHITE
                auto pt_bb_w = pt_bb & w_p;
                while (pt_bb_w)
                {
                    phase -= eval::hce::k_PHASE_WEIGHTS[pt];
                    (void) pt_bb_w.popLsb();
                }
                // BLACK
                auto pt_bb_b = pt_bb & b_p;
                while (pt_bb_b)
                {
                    phase -= eval::hce::k_PHASE_WEIGHTS[pt];
                    (void) pt_bb_b.popLsb();
                }
            }

            return phase;
        };

        Position pos;

        // Startpos
        pos.setFen("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");

        I32 phase = raw_phase(pos);
        phase     = eval::hce::stdPhase(phase);

        CHECK(phase == 0);
        CHECK(phase == eval::hce::posPhase(pos));

        // End Game
        pos.setFen("4k3/8/8/8/8/8/8/4K3 w - - 0 1");

        phase = raw_phase(pos);
        phase = eval::hce::stdPhase(phase);

        CHECK(phase == 256);
        CHECK(phase == eval::hce::posPhase(pos));
    }

    TEST_CASE("eval::scaleEval") {
        CHECK(eval::hce::scaleEval(100, 200, 0) == 100);
        CHECK(eval::hce::scaleEval(100, 200, 256) == 200);
        CHECK(eval::hce::scaleEval(100, 200, 128) == 150);
    }

    TEST_CASE("eval::phase_factors") {
        Position pos;

        // Startpos
        pos.setFen("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");

        I32    phase = eval::hce::posPhase(pos);
        double w_eg  = phase / 256.0;
        double w_mg  = 1.0 - w_eg;

        CHECK(w_mg == 1.0);
        CHECK(w_eg == 0.0);

        // Eng Game
        pos.setFen("4k3/8/8/8/8/8/8/4K3 w - - 0 1");

        phase = eval::hce::posPhase(pos);
        w_eg  = phase / 256.0;
        w_mg  = 1.0 - w_eg;

        CHECK(w_mg == 0.0);
        CHECK(w_eg == 1.0);
    }

    TEST_CASE("eval::mg_eg_score") {
        I32 score = eval::hce::s(100, 200);

        CHECK(eval::hce::mgScore(score) == 100);
        CHECK(eval::hce::egScore(score) == 200);
    }
}
