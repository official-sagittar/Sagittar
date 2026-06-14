#include "eval.h"
#include "commons/utils.h"
#include "core/bitboard.h"
#include "eval/hce/defs.h"

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
    }

    Score evaluate(const Position& pos) {
        i32   phase   = TOTAL_PHASE;
        Score eval_mg = 0;
        Score eval_eg = 0;

        const auto w_p = pos.pieces(Color::WHITE);
        const auto b_p = pos.pieces(Color::BLACK);

        for (int pt = PieceType::PAWN; pt <= PieceType::KING; pt++)
        {
            const auto pt_bb = pos.pieces(static_cast<PieceType>(pt));
            // WHITE
            auto pt_bb_w = pt_bb & w_p;
            while (pt_bb_w)
            {
                phase -= PHASE_WEIGHTS[pt];
                const auto sq = utils::bitScanForward(&pt_bb_w);
                eval_mg += psqt_w[pt][MG][sq];
                eval_eg += psqt_w[pt][EG][sq];
            }
            // BLACK
            auto pt_bb_b = pt_bb & b_p;
            while (pt_bb_b)
            {
                phase -= PHASE_WEIGHTS[pt];
                const auto sq = utils::bitScanForward(&pt_bb_b);
                eval_mg -= psqt_b[pt][MG][sq];
                eval_eg -= psqt_b[pt][EG][sq];
            }
        }

        // Doubled / Tripled Pawns
        const auto pawn_bb   = pos.pieces(PieceType::PAWN);
        i32        doubled_w = 0;
        i32        doubled_b = 0;
        for (int f = 0; f < 8; f++)
        {
            const auto file_bb = FILE_A_BB << f;
            const i32  w_cnt   = utils::bitCount1s(pawn_bb & w_p & file_bb);
            const i32  b_cnt   = utils::bitCount1s(pawn_bb & b_p & file_bb);
            doubled_w += std::max(0, w_cnt - 1);
            doubled_b += std::max(0, b_cnt - 1);
        }
        eval_mg += mg_score(DOUBLED_PAWN) * doubled_w;
        eval_eg += eg_score(DOUBLED_PAWN) * doubled_w;
        eval_mg -= mg_score(DOUBLED_PAWN) * doubled_b;
        eval_eg -= eg_score(DOUBLED_PAWN) * doubled_b;

        // Bishop Pair
        const auto b_bb          = pos.pieces(PieceType::BISHOP);
        const auto bishop_pair_w = (utils::bitCount1s(b_bb & w_p) > 1);
        const auto bishop_pair_b = (utils::bitCount1s(b_bb & b_p) > 1);
        // White
        eval_mg += mg_score(BISHOP_PAIR) * bishop_pair_w;
        eval_eg += eg_score(BISHOP_PAIR) * bishop_pair_w;
        // Black
        eval_mg -= mg_score(BISHOP_PAIR) * bishop_pair_b;
        eval_eg -= eg_score(BISHOP_PAIR) * bishop_pair_b;

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

            return ((utils::bitCount1s(w_minors_bb) <= 1) && (utils::bitCount1s(b_minors_bb) <= 1));
        }

        return false;
    }

}
