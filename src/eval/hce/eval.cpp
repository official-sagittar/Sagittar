#include "eval.h"
#include "commons/utils.h"
#include "eval/hce/defs.h"

namespace sagittar::eval::hce {

    namespace {

        constexpr std::array<std::array<PSQT, 2>, 6> psqt_b = []() {
            std::array<std::array<PSQT, 2>, 6> table{};

            for (int pt = PieceType::PAWN; pt <= PieceType::KING; pt++)
            {
                for (int sq = Square::A1; sq <= Square::H8; sq++)
                {
                    table[index(pt)][index(MG)][index(sq)] =
                      mg_score(PIECE_SCORES[index(pt)])
                      + mg_score(PSQT_SCORES[index(pt)][index(sq)]);
                    table[index(pt)][index(EG)][index(sq)] =
                      eg_score(PIECE_SCORES[index(pt)])
                      + eg_score(PSQT_SCORES[index(pt)][index(sq)]);
                }
            }

            return table;
        }();

        constexpr std::array<std::array<PSQT, 2>, 6> psqt_w = []() {
            std::array<std::array<PSQT, 2>, 6> table{};

            for (int pt = PieceType::PAWN; pt <= PieceType::KING; pt++)
            {
                for (int sq = Square::A1; sq <= Square::H8; sq++)
                {
                    table[index(pt)][index(MG)][index(sq)] =
                      psqt_b[index(pt)][index(MG)][SQUARES_MIRRORED[index(sq)]];
                    table[index(pt)][index(EG)][index(sq)] =
                      psqt_b[index(pt)][index(EG)][index(SQUARES_MIRRORED[index(sq)])];
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
                    eval_mg += psqt_w[index(PieceType::PAWN)][index(MG)][index(sq)];
                    eval_eg += psqt_w[index(PieceType::PAWN)][index(EG)][index(sq)];
                }
            }

            // BLACK
            {
                auto b_p_bb = pos.pieces(Color::BLACK, PieceType::PAWN);
                while (b_p_bb)
                {
                    const auto sq = b_p_bb.pop_lsb();
                    eval_mg -= psqt_b[index(PieceType::PAWN)][index(MG)][index(sq)];
                    eval_eg -= psqt_b[index(PieceType::PAWN)][index(EG)][index(sq)];
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
                phase -= PHASE_WEIGHTS[index(pt)];
                const auto sq = pt_bb_w.pop_lsb();
                eval_mg += psqt_w[index(pt)][index(MG)][index(sq)];
                eval_eg += psqt_w[index(pt)][index(EG)][index(sq)];
            }
            // BLACK
            auto pt_bb_b = pt_bb & b_p;
            while (pt_bb_b)
            {
                phase -= PHASE_WEIGHTS[index(pt)];
                const auto sq = pt_bb_b.pop_lsb();
                eval_mg -= psqt_b[index(pt)][index(MG)][index(sq)];
                eval_eg -= psqt_b[index(pt)][index(EG)][index(sq)];
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
