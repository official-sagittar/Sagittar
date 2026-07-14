#include "eval.h"
#include "commons/utils.h"
#include "eval/hce/defs.h"
#include <array>
#include <cstddef>
#include <utility>

namespace sagittar::eval::hce {

    namespace {

        constexpr std::array<std::array<PSQT, 2>, 7> psqt_b = []() {
            std::array<std::array<PSQT, 2>, 7> table{};

            for (int pt = PieceType::PIECE_TYPE_INVALID; pt <= PieceType::KING; pt++)
            {
                for (int sq = Square::A1; sq <= Square::H8; sq++)
                {
                    table[pt][MG][sq] = mg_score(PIECE_SCORES[pt]) + mg_score(PSQT_SCORES[pt][sq]);
                    table[pt][EG][sq] = eg_score(PIECE_SCORES[pt]) + eg_score(PSQT_SCORES[pt][sq]);
                }
            }

            return table;
        }();

        constexpr std::array<std::array<PSQT, 2>, 7> psqt_w = []() {
            std::array<std::array<PSQT, 2>, 7> table{};

            for (int pt = PieceType::PIECE_TYPE_INVALID; pt <= PieceType::KING; pt++)
            {
                for (int sq = Square::A1; sq <= Square::H8; sq++)
                {
                    table[pt][MG][sq] = psqt_b[pt][MG][SQUARES_MIRRORED[sq]];
                    table[pt][EG][sq] = psqt_b[pt][EG][SQUARES_MIRRORED[sq]];
                }
            }

            return table;
        }();

        std::pair<Score, Score> evaluate_pawns(const Position& pos) {
            Score eval_mg = 0;
            Score eval_eg = 0;

            // WHITE
            {
                auto w_p_bb = pos.pieces(Color::WHITE, PieceType::PAWN);
                while (w_p_bb)
                {
                    const auto sq = w_p_bb.pop_lsb();
                    eval_mg += psqt_w[PieceType::PAWN][MG][sq];
                    eval_eg += psqt_w[PieceType::PAWN][EG][sq];
                }
            }

            // BLACK
            {
                auto b_p_bb = pos.pieces(Color::BLACK, PieceType::PAWN);
                while (b_p_bb)
                {
                    const auto sq = b_p_bb.pop_lsb();
                    eval_mg -= psqt_b[PieceType::PAWN][MG][sq];
                    eval_eg -= psqt_b[PieceType::PAWN][EG][sq];
                }
            }

            return std::make_pair(eval_mg, eval_eg);
        }

        class PawnCache {
           public:
            [[nodiscard]] std::pair<Score, Score> probe(const Position& pos) noexcept {
                const u64 pawn_key = pos.pawn_key();

                auto& entry = m_entries[static_cast<std::size_t>(pawn_key % SIZE)];

                if (entry.pawn_key != pawn_key)
                {
                    entry.eval     = evaluate_pawns(pos);
                    entry.pawn_key = pawn_key;
                }

                return entry.eval;
            }

           private:
            static constexpr std::size_t SIZE = 128 * 1024;

            struct PawnEvalEntry {
                u64                     pawn_key{};
                std::pair<Score, Score> eval{};
            };

            std::array<PawnEvalEntry, SIZE> m_entries{};
        };

    }

    Score evaluate(const Position& pos) {
        i32   phase   = TOTAL_PHASE;
        Score eval_mg = 0;
        Score eval_eg = 0;

        // Evaluate Pawns
        static PawnCache pawn_cache{};

        const auto pawn_eval = pawn_cache.probe(pos);
        eval_mg              = pawn_eval.first;
        eval_eg              = pawn_eval.second;

        // Evaluate Pieces
        const auto w_p = pos.pieces(Color::WHITE);
        const auto b_p = pos.pieces(Color::BLACK);

        for (int pt = PieceType::KNIGHT; pt <= PieceType::KING; pt++)
        {
            const auto pt_bb = pos.pieces(static_cast<PieceType>(pt));
            // WHITE
            auto pt_bb_w = pt_bb & w_p;
            while (pt_bb_w)
            {
                phase -= PHASE_WEIGHTS[pt];
                const auto sq = pt_bb_w.pop_lsb();
                eval_mg += psqt_w[pt][MG][sq];
                eval_eg += psqt_w[pt][EG][sq];
            }
            // BLACK
            auto pt_bb_b = pt_bb & b_p;
            while (pt_bb_b)
            {
                phase -= PHASE_WEIGHTS[pt];
                const auto sq = pt_bb_b.pop_lsb();
                eval_mg -= psqt_b[pt][MG][sq];
                eval_eg -= psqt_b[pt][EG][sq];
            }
        }

        phase = std_phase(phase);
#ifdef DEBUG
        assert(phase == pos_phase(pos));
#endif

        Score eval = scale_eval(eval_mg, eval_eg, phase);

        const auto stm = 1 - (2 * pos.stm());

        eval *= stm;

        // Tempo Bonus
        const Score tempo_bonus = scale_eval(mg_score(TEMPO_BONUS), eg_score(TEMPO_BONUS), phase);
        eval += tempo_bonus;

        return eval;
    }

    bool isEndGame(const Position& pos) {
        const auto queens = pos.pieceCount(PieceType::QUEEN);

        if (queens == 0)
        {
            return true;
        }

        if ((queens == 2) && (pos.pieceCount(PieceType::ROOK) == 0))
        {
            const auto w_minors_bb = pos.pieces(Color::WHITE, PieceType::KNIGHT, PieceType::BISHOP);
            const auto b_minors_bb = pos.pieces(Color::BLACK, PieceType::KNIGHT, PieceType::BISHOP);

            return ((w_minors_bb.count() <= 1) && (b_minors_bb.count() <= 1));
        }

        return false;
    }

}
