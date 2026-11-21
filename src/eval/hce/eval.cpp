#include "eval.h"
#include "commons/utils.h"
#include "eval/hce/defs.h"

namespace sagittar::eval::hce {

    namespace {

        constexpr std::array<std::array<PSQT, 2>, 6> k_PSQT_B = []() {
            std::array<std::array<PSQT, 2>, 6> table{};

            for (int pt = PieceType::PAWN; pt <= PieceType::KING; pt++)
            {
                for (int sq = Square::A1; sq <= Square::H8; sq++)
                {
                    table[pt][MG][sq] =
                      mgScore(k_PIECE_SCORES[pt]) + mgScore(k_PSQT_SCORES[pt][sq]);
                    table[pt][EG][sq] =
                      egScore(k_PIECE_SCORES[pt]) + egScore(k_PSQT_SCORES[pt][sq]);
                }
            }

            return table;
        }();

        constexpr std::array<std::array<PSQT, 2>, 6> k_PSQT_W = []() {
            std::array<std::array<PSQT, 2>, 6> table{};

            for (int pt = PieceType::PAWN; pt <= PieceType::KING; pt++)
            {
                for (int sq = Square::A1; sq <= Square::H8; sq++)
                {
                    table[pt][MG][sq] = k_PSQT_B[pt][MG][k_SQUARES_MIRRORED[sq]];
                    table[pt][EG][sq] = k_PSQT_B[pt][EG][k_SQUARES_MIRRORED[sq]];
                }
            }

            return table;
        }();

        std::pair<Score, Score> evaluatePawns(const Position& pos) {
            Score eval_mg = 0;
            Score eval_eg = 0;

            // WHITE
            {
                auto w_p_bb = pos.pieces(Color::WHITE, PieceType::PAWN);
                while (w_p_bb)
                {
                    const auto sq = w_p_bb.popLsb();
                    eval_mg += k_PSQT_W[PieceType::PAWN][MG][sq];
                    eval_eg += k_PSQT_W[PieceType::PAWN][EG][sq];
                }
            }

            // BLACK
            {
                auto b_p_bb = pos.pieces(Color::BLACK, PieceType::PAWN);
                while (b_p_bb)
                {
                    const auto sq = b_p_bb.popLsb();
                    eval_mg -= k_PSQT_B[PieceType::PAWN][MG][sq];
                    eval_eg -= k_PSQT_B[PieceType::PAWN][EG][sq];
                }
            }

            return std::make_pair(eval_mg, eval_eg);
        }

        class PawnCache {
           public:
            [[nodiscard]] std::pair<Score, Score> probe(const Position& pos) noexcept {
                const U64 pawn_key = pos.pawnKey();

                auto& entry = m_entries[static_cast<std::size_t>(pawn_key % k_SIZE)];

                if (entry.pawn_key != pawn_key)
                {
                    entry.eval     = evaluatePawns(pos);
                    entry.pawn_key = pawn_key;
                }

                return entry.eval;
            }

           private:
            static constexpr std::size_t k_SIZE = 128 * 1024;

            struct PawnEvalEntry {
                U64                     pawn_key{};
                std::pair<Score, Score> eval{};
            };

            std::array<PawnEvalEntry, k_SIZE> m_entries{};
        };

    }

    Score evaluate(const Position& pos) {
        I32   phase   = k_TOTAL_PHASE;
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
            const PieceType piece_type = static_cast<PieceType>(pt);
            const auto      pt_bb      = pos.pieces(piece_type);
            // WHITE
            auto pt_bb_w = pt_bb & w_p;
            while (pt_bb_w)
            {
                phase -= k_PHASE_WEIGHTS[pt];
                const auto sq = pt_bb_w.popLsb();
                eval_mg += k_PSQT_W[pt][MG][sq];
                eval_eg += k_PSQT_W[pt][EG][sq];

                // Mobility
                if (piece_type != PieceType::KING)
                {
                    const auto mobility_count =
                      mobility<Color::WHITE>(pos, piece_type, static_cast<Square>(sq));
                    const auto mobility_bonus = k_MOBILITY_BONUS[pt - 1][mobility_count];
                    eval_mg += mgScore(mobility_bonus);
                    eval_eg += egScore(mobility_bonus);
                }
            }
            // BLACK
            auto pt_bb_b = pt_bb & b_p;
            while (pt_bb_b)
            {
                phase -= k_PHASE_WEIGHTS[pt];
                const auto sq = pt_bb_b.popLsb();
                eval_mg -= k_PSQT_B[pt][MG][sq];
                eval_eg -= k_PSQT_B[pt][EG][sq];

                // Mobility
                if (piece_type != PieceType::KING)
                {
                    const auto mobility_count =
                      mobility<Color::BLACK>(pos, piece_type, static_cast<Square>(sq));
                    const auto mobility_bonus = k_MOBILITY_BONUS[pt - 1][mobility_count];
                    eval_mg -= mgScore(mobility_bonus);
                    eval_eg -= egScore(mobility_bonus);
                }
            }
        }

        phase = stdPhase(phase);
#ifdef DEBUG
        assert(phase == posPhase(pos));
#endif

        // Bishop pair
        const auto b_bb   = pos.pieces(PieceType::BISHOP);
        const auto w_b_bb = b_bb & w_p;
        const auto b_b_bb = b_bb & b_p;
        if (w_b_bb.hasMultiple())
        {
            eval_mg += mgScore(k_BISHOP_PAIR);
            eval_eg += egScore(k_BISHOP_PAIR);
        }
        if (b_b_bb.hasMultiple())
        {
            eval_mg -= mgScore(k_BISHOP_PAIR);
            eval_eg -= egScore(k_BISHOP_PAIR);
        }

        Score eval = scaleEval(eval_mg, eval_eg, phase);

        const auto stm = 1 - (2 * pos.stm());

        eval *= stm;

        // Tempo Bonus
        const Score tempo_bonus = scaleEval(mgScore(k_TEMPO_BONUS), egScore(k_TEMPO_BONUS), phase);
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
