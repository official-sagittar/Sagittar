#include "eval.h"

namespace sagittar {

    namespace eval {

        enum GamePhase {
            MG,
            EG
        };

        constexpr i32 S(const i32 mg, const i32 eg) {
            return static_cast<i32>(static_cast<u32>(eg) << 16) + mg;
        }

        constexpr i32 mg_score(const i32 score) { return static_cast<i16>(score); }

        constexpr i32 eg_score(const i32 score) { return static_cast<i16>((score + 0x8000) >> 16); }

        // https://www.chessprogramming.org/PeSTO%27s_Evaluation_Function
        // clang-format off
        static const i32 PIECE_SCORES[6] = {
            S(82, 94),      // PAWN
            S(337, 281),    // KNIGHT
            S(365, 297),    // BISHOP
            S(477, 512),    // ROOK
            S(1025, 936),   // QUEEN
            0               // KING
        };

        static const i32 PSQT_SCORES[6][64] = {
            {       // PAWN
                S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0),
                S(98, 178), S(134, 173), S(61, 158), S(95, 134), S(68, 147), S(126, 132), S(34, 165), S(-11, 187),
                S(-6, 94), S(7, 100), S(26, 85), S(31, 67), S(65, 56), S(56, 53), S(25, 82), S(-20, 84),
                S(-14, 32), S(13, 24), S(6, 13), S(21, 5), S(23, -2), S(12, 4), S(17, 17), S(-23, 17),
                S(-27, 13), S(-2, 9), S(-5, -3), S(12, -7), S(17, -7), S(6, -8), S(10, 3), S(-25, -1),
                S(-26, 4), S(-4, 7), S(-4, -6), S(-10, 1), S(3, 0), S(3, -5), S(33, -1), S(-12, -8),
                S(-35, 13), S(-1, 8), S(-20, 8), S(-23, 10), S(-15, 13), S(24, 0), S(38, 2), S(-22, -7),
                S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0)
            }, {    // KNIGHT
                S(-167, -58), S(-89, -38), S(-34, -13), S(-49, -28), S(61, -31), S(-97, -27), S(-15, -63), S(-107, -99),
                S(-73, -25), S(-41, -8), S(72, -25), S(36, -2), S(23, -9), S(62, -25), S(7, -24), S(-17, -52),
                S(-47, -24), S(60, -20), S(37, 10), S(65, 9), S(84, -1), S(129, -9), S(73, -19), S(44, -41),
                S(-9, -17), S(17, 3), S(19, 22), S(53, 22), S(37, 22), S(69, 11), S(18, 8), S(22, -18),
                S(-13, -18), S(4, -6), S(16, 16), S(13, 25), S(28, 16), S(19, 17), S(21, 4), S(-8, -18),
                S(-23, -23), S(-9, -3), S(12, -1), S(10, 15), S(19, 10), S(17, -3), S(25, -20), S(-16, -22),
                S(-29, -42), S(-53, -20), S(-12, -10), S(-3, -5), S(-1, -2), S(18, -20), S(-14, -23), S(-19, -44),
                S(-105, -29), S(-21, -51), S(-58, -23), S(-33, -15), S(-17, -22), S(-28, -18), S(-19, -50), S(-23, -64)
            }, {    // BISHOP
                S(-29, -14), S(4, -21), S(-82, -11), S(-37, -8), S(-25, -7), S(-42, -9), S(7, -17), S(-8, -24),
                S(-26, -8), S(16, -4), S(-18, 7), S(-13, -12), S(30, -3), S(59, -13), S(18, -4), S(-47, -14),
                S(-16, 2), S(37, -8), S(43, 0), S(40, -1), S(35, -2), S(50, 6), S(37, 0), S(-2, 4),
                S(-4, -3), S(5, 9), S(19, 12), S(50, 9), S(37, 14), S(37, 10), S(7, 3), S(-2, 2),
                S(-6, -6), S(13, 3), S(13, 13), S(26, 19), S(34, 7), S(12, 10), S(10, -3), S(4, -9),
                S(0, -12), S(15, -3), S(15, 8), S(15, 10), S(14, 13), S(27, 3), S(18, -7), S(10, -15),
                S(4, -14), S(15, -18), S(16, -7), S(0, -1), S(7, 4), S(21, -9), S(33, -15), S(1, -27),
                S(-33, -23), S(-3, -9), S(-14, -23), S(-21, -5), S(-13, -9), S(-12, -16), S(-39, -5), S(-21, -17)
            }, {    // ROOK
                S(32, 13), S(42, 10), S(32, 18), S(51, 15), S(63, 12), S(9, 12), S(31, 8), S(43, 5),
                S(27, 11), S(32, 13), S(58, 13), S(62, 11), S(80, -3), S(67, 3), S(26, 8), S(44, 3),
                S(-5, 7), S(19, 7), S(26, 7), S(36, 5), S(17, 4), S(45, -3), S(61, -5), S(16, -3),
                S(-24, 4), S(-11, 3), S(7, 13), S(26, 1), S(24, 2), S(35, 1), S(-8, -1), S(-20, 2),
                S(-36, 3), S(-26, 5), S(-12, 8), S(-1, 4), S(9, -5), S(-7, -6), S(6, -8), S(-23, -11),
                S(-45, -4), S(-25, 0), S(-16, -5), S(-17, -1), S(3, -7), S(0, -12), S(-5, -8), S(-33, -16),
                S(-44, -6), S(-16, -6), S(-20, 0), S(-9, 2), S(-1, -9), S(11, -9), S(-6, -11), S(-71, -3),
                S(-19, -9), S(-13, 2), S(1, 3), S(17, -1), S(16, -5), S(7, -13), S(-37, 4), S(-26, -20)
            }, {    // QUEEN
                S(-28, -9), S(0, 22), S(29, 22), S(12, 27), S(59, 27), S(44, 19), S(43, 10), S(45, 20),
                S(-24, -17), S(-39, 20), S(-5, 32), S(1, 41), S(-16, 58), S(57, 25), S(28, 30), S(54, 0),
                S(-13, -20), S(-17, 6), S(7, 9), S(8, 49), S(29, 47), S(56, 35), S(47, 19), S(57, 9),
                S(-27, 3), S(-27, 22), S(-16, 24), S(-16, 45), S(-1, 57), S(17, 40), S(-2, 57), S(1, 36),
                S(-9, -18), S(-26, 28), S(-9, 19), S(-10, 47), S(-2, 31), S(-4, 34), S(3, 39), S(-3, 23),
                S(-14, -16), S(2, -27), S(-11, 15), S(-2, 6), S(-5, 9), S(2, 17), S(14, 10), S(5, 5),
                S(-35, -22), S(-8, -23), S(11, -30), S(2, -16), S(8, -16), S(15, -23), S(-3, -36), S(1, -32),
                S(-1, -33), S(-18, -28), S(-9, -22), S(10, -43), S(-15, -5), S(-25, -32), S(-31, -20), S(-50, -41)
            }, {    // KING
                S(-65, -74), S(23, -35), S(16, -18), S(-15, -18), S(-56, -11), S(-34, 15), S(2, 4), S(13, -17),
                S(29, -12), S(-1, 17), S(-20, 14), S(-7, 17), S(-8, 17), S(-4, 38), S(-38, 23), S(-29, 11),
                S(-9, 10), S(24, 17), S(2, 23), S(-16, 15), S(-20, 20), S(6, 45), S(22, 44), S(-22, 13),
                S(-17, -8), S(-20, 22), S(-12, 24), S(-27, 27), S(-30, 26), S(-25, 33), S(-14, 26), S(-36, 3),
                S(-49, -18), S(-1, -4), S(-27, 21), S(-39, 24), S(-46, 27), S(-44, 23), S(-33, 9), S(-51, -11),
                S(-14, -19), S(-14, -3), S(-22, 11), S(-46, 21), S(-44, 23), S(-30, 16), S(-15, 7), S(-27, -9),
                S(1, -27), S(7, -11), S(-8, 4), S(-64, 13), S(-43, 14), S(-16, 4), S(9, -5), S(8, -17),
                S(-15, -53), S(36, -34), S(12, -21), S(-54, -11), S(8, -28), S(-28, -14), S(24, -24), S(14, -43)
            }
        };
        // clang-format on

        static const i32 PHASE_WEIGHTS[6] = {0, 1, 1, 2, 4, 0};
        static const i32 TOTAL_PHASE      = 24;
        static const i32 TEMPO_BONUS      = S(15, 3);
        static i32       PSQT[6][64][2];

        void initialize() {
            for (int p = 0; p < 6; p++)
            {
                for (int sq = 0; sq < 64; sq++)
                {
                    PSQT[p][sq][MG] = mg_score(PIECE_SCORES[p]) + mg_score(PSQT_SCORES[p][sq]);
                    PSQT[p][sq][EG] = eg_score(PIECE_SCORES[p]) + eg_score(PSQT_SCORES[p][sq]);
                }
            }
        }

        bool isInsufficientMaterial(const board::Board& board) {
            const u8 wQ = board.getPieceCount(Piece::WHITE_QUEEN);
            const u8 bQ = board.getPieceCount(Piece::BLACK_QUEEN);

            const u8 wR = board.getPieceCount(Piece::WHITE_ROOK);
            const u8 bR = board.getPieceCount(Piece::BLACK_ROOK);

            const u8 wP = board.getPieceCount(Piece::WHITE_PAWN);
            const u8 bP = board.getPieceCount(Piece::BLACK_PAWN);

            const bool qrp = ((wQ + bQ) > 0) || ((wR + bR) > 0) || ((wP + bP) > 0);

            if (qrp)
            {
                return false;
            }

            const u8 whitePiecesCount = board.getPiecesCount(Color::WHITE);
            const u8 blackPiecesCount = board.getPiecesCount(Color::BLACK);

            // King vs King
            const bool kvk = (whitePiecesCount == 1) && (blackPiecesCount == 1);

            // King + Bishop vs King
            const u8   wB   = board.getPieceCount(Piece::WHITE_BISHOP);
            const u8   bB   = board.getPieceCount(Piece::BLACK_BISHOP);
            const bool kbvk = (whitePiecesCount == 2 && wB == 1 && blackPiecesCount == 1)
                           || (blackPiecesCount == 2 && bB == 1 && whitePiecesCount == 1);

            // King + Knight vs King
            const u8   wN   = board.getPieceCount(Piece::WHITE_KNIGHT);
            const u8   bN   = board.getPieceCount(Piece::BLACK_KNIGHT);
            const bool knvk = (whitePiecesCount == 2 && wN == 1 && blackPiecesCount == 1)
                           || (blackPiecesCount == 2 && bN == 1 && whitePiecesCount == 1);

            // King + Bishop vs King + Bishop (both on same color squares)
            const bool kbvkb =
              ((whitePiecesCount == 2) && (blackPiecesCount == 2) && (wB == 1) && (bB == 1));

            const auto wB_bb = board.getBitboard(Piece::WHITE_BISHOP);
            const auto bB_bb = board.getBitboard(Piece::BLACK_BISHOP);

            const bool bsamecolor = (((wB_bb & board::MASK_LIGHT_SQUARES) != 0)
                                     && ((bB_bb & board::MASK_LIGHT_SQUARES) != 0))
                                 || (((wB_bb & board::MASK_DARK_SQUARES) != 0)
                                     && ((bB_bb & board::MASK_DARK_SQUARES) != 0));

            return kvk | kbvk | knvk | (kbvkb & bsamecolor);
        }

        i32 evaluateBoard(const board::Board& board) {
            i32 phase   = TOTAL_PHASE;
            i32 eval_mg = 0;
            i32 eval_eg = 0;

            for (u8 sq = A1; sq <= H8; sq++)
            {
                const Piece piece = board.getPiece(static_cast<Square>(sq));
                if (piece == Piece::NO_PIECE)
                {
                    continue;
                }

                const PieceType ptype  = pieceTypeOf(piece);
                const Color     pcolor = pieceColorOf(piece);

                phase -= PHASE_WEIGHTS[ptype - 1];

                switch (pcolor)
                {
                    case Color::WHITE :
                        eval_mg += PSQT[ptype - 1][SQUARES_MIRRORED[sq]][MG];
                        eval_eg += PSQT[ptype - 1][SQUARES_MIRRORED[sq]][EG];
                        break;
                    case Color::BLACK :
                        eval_mg -= PSQT[ptype - 1][sq][MG];
                        eval_eg -= PSQT[ptype - 1][sq][EG];
                        break;
                }
            }

            phase = (phase * 256 + (TOTAL_PHASE / 2)) / TOTAL_PHASE;

            i32 eval = ((eval_mg * (256 - phase)) + (eval_eg * phase)) / 256;

            const i8 stm = 1 - (2 * board.getActiveColor());
#ifdef DEBUG
            if (board.getActiveColor() == Color::WHITE)
            {
                assert(stm == 1);
            }
            else
            {
                assert(stm == -1);
            }
#endif

            // Tempo Bonus
            const i32 tempo_bonus =
              ((mg_score(TEMPO_BONUS) * (256 - phase)) + (eg_score(TEMPO_BONUS) * phase)) / 256;
            eval += tempo_bonus * stm;

            eval *= stm;

            return eval;
        }

        bool isEndGame(const board::Board& board) {
            const u8 wQ = board.getPieceCount(Piece::WHITE_QUEEN);
            const u8 bQ = board.getPieceCount(Piece::BLACK_QUEEN);

            const u8 wR = board.getPieceCount(Piece::WHITE_ROOK);
            const u8 bR = board.getPieceCount(Piece::BLACK_ROOK);

            const u8 wB = board.getPieceCount(Piece::WHITE_BISHOP);
            const u8 bB = board.getPieceCount(Piece::BLACK_BISHOP);

            const u8 wN = board.getPieceCount(Piece::WHITE_KNIGHT);
            const u8 bN = board.getPieceCount(Piece::BLACK_KNIGHT);

            bool is_end_game = false;

            if (wQ == 0 && bQ == 0)
            {
                is_end_game = true;
            }
            else if (wQ == 1 && bQ == 1 && wR == 0 && bR == 0)
            {
                if ((wN + wB) <= 1 && (bN + bB) <= 1)
                {
                    is_end_game = true;
                }
            }

            return is_end_game;
        }

    }

}
