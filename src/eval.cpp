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

        // https://www.chessprogramming.org/Simplified_Evaluation_Function
        // clang-format off
        static const i32 PIECE_SCORES[6] = {
            S(100, 100),    // PAWN
            S(310, 310),    // KNIGHT
            S(320, 320),    // BISHOP
            S(500, 500),    // ROOK
            S(900, 900),    // QUEEN
            0               // KING
        };

        static const i32 PSQT_SCORES[6][64] = {
            {       // PAWN
                S(0, 0),      S(0, 0),    S(0, 0),     S(0, 0),     S(0, 0),     S(0, 0),     S(0, 0),     S(0, 0),
                S(50, 50),    S(50, 50),  S(50, 50),   S(50, 50),   S(50, 50),   S(50, 50),   S(50, 50),   S(50, 50),
                S(10, 10),    S(10, 10),  S(20, 20),   S(30, 30),   S(30, 30),   S(20, 20),   S(10, 10),   S(10, 10),
                S(5, 5),      S(5, 5),    S(10, 10),   S(25, 25),   S(25, 25),   S(10, 10),   S(5, 5),     S(5, 5),
                S(0, 0),      S(0, 0),    S(0, 0),     S(20, 20),   S(20, 20),   S(0, 0),     S(0, 0),     S(0, 0),
                S(5, 5),      S(-5, -5),  S(-10, -10), S(0, 0),     S(0, 0),     S(-10, -10), S(-5, -5),   S(5, 5),
                S(5, 5),      S(10, 10),  S(10, 10),   S(-20, -20), S(-20, -20), S(10, 10),   S(10, 10),   S(5, 5),
                S(0, 0),      S(0, 0),    S(0, 0),     S(0, 0),     S(0, 0),     S(0, 0),     S(0, 0),     S(0, 0)
            }, {    // KNIGHT
                S(-50, -50), S(-40, -40), S(-30, -30), S(-30, -30), S(-30, -30), S(-30, -30), S(-40, -40), S(-50, -50),
                S(-40, -40), S(-20, -20), S(0, 0),     S(0, 0),     S(0, 0),     S(0, 0),     S(-20, -20), S(-40, -40),
                S(-30, -30), S(0, 0),     S(10, 10),   S(15, 15),   S(15, 15),   S(10, 10),   S(0, 0),     S(-30, -30),
                S(-30, -30), S(5, 5),     S(15, 15),   S(20, 20),   S(20, 20),   S(15, 15),   S(5, 5),     S(-30, -30),
                S(-30, -30), S(0, 0),     S(15, 15),   S(20, 20),   S(20, 20),   S(15, 15),   S(0, 0),     S(-30, -30),
                S(-30, -30), S(5, 5),     S(10, 10),   S(15, 15),   S(15, 15),   S(10, 10),   S(5, 5),     S(-30, -30),
                S(-40, -40), S(-20, -20), S(0, 0),     S(5, 5),     S(5, 5),     S(0, 0),     S(-20, -20), S(-40, -40),
                S(-50, -50), S(-40, -40), S(-30, -30), S(-30, -30), S(-30, -30), S(-30, -30), S(-40, -40), S(-50, -50)
            }, {    // BISHOP
                S(-20, -20), S(-10, -10), S(-10, -10), S(-10, -10), S(-10, -10), S(-10, -10), S(-10, -10), S(-20, -20),
                S(-10, -10), S(0, 0),     S(0, 0),     S(0, 0),     S(0, 0),     S(0, 0),     S(0, 0),     S(-10, -10),
                S(-10, -10), S(0, 0),     S(5, 5),     S(10, 10),   S(10, 10),   S(5, 5),     S(0, 0),     S(-10, -10),
                S(-10, -10), S(5, 5),     S(5, 5),     S(10, 10),   S(10, 10),   S(5, 5),     S(5, 5),     S(-10, -10),
                S(-10, -10), S(0, 0),     S(10, 10),   S(10, 10),   S(10, 10),   S(10, 10),   S(0, 0),     S(-10, -10),
                S(-10, -10), S(10, 10),   S(10, 10),   S(10, 10),   S(10, 10),   S(10, 10),   S(10, 10),   S(-10, -10),
                S(-10, -10), S(5, 5),     S(0, 0),     S(0, 0),     S(0, 0),     S(0, 0),     S(5, 5),     S(-10, -10),
                S(-20, -20), S(-10, -10), S(-10, -10), S(-10, -10), S(-10, -10), S(-10, -10), S(-10, -10), S(-20, -20)
            }, {    // ROOK
                S(0, 0),     S(0, 0),     S(0, 0),     S(0, 0),     S(0, 0),     S(0, 0),     S(0, 0),     S(0, 0),
                S(5, 5),     S(10, 10),   S(10, 10),   S(10, 10),   S(10, 10),   S(10, 10),   S(10, 10),   S(5, 5),
                S(-5, -5),   S(0, 0),     S(0, 0),     S(0, 0),     S(0, 0),     S(0, 0),     S(0, 0),     S(-5, -5),
                S(-5, -5),   S(0, 0),     S(0, 0),     S(0, 0),     S(0, 0),     S(0, 0),     S(0, 0),     S(-5, -5),
                S(-5, -5),   S(0, 0),     S(0, 0),     S(0, 0),     S(0, 0),     S(0, 0),     S(0, 0),     S(-5, -5),
                S(-5, -5),   S(0, 0),     S(0, 0),     S(0, 0),     S(0, 0),     S(0, 0),     S(0, 0),     S(-5, -5),
                S(-5, -5),   S(0, 0),     S(0, 0),     S(0, 0),     S(0, 0),     S(0, 0),     S(0, 0),     S(-5, -5),
                S(0, 0),     S(0, 0),     S(0, 0),     S(5, 5),     S(5, 5),     S(0, 0),     S(0, 0),     S(0, 0)
            }, {    // QUEEN
                S(-20, -20), S(-10, -10), S(-10, -10), S(-5, -5),   S(-5, -5),   S(-10, -10), S(-10, -10), S(-20, -20),
                S(-10, -10), S(0, 0),     S(0, 0),     S(0, 0),     S(0, 0),     S(0, 0),     S(0, 0),     S(-10, -10),
                S(-10, -10), S(0, 0),     S(5, 5),     S(5, 5),     S(5, 5),     S(5, 5),     S(0, 0),     S(-10, -10),
                S(-5, -5),   S(0, 0),     S(5, 5),     S(5, 5),     S(5, 5),     S(5, 5),     S(0, 0),     S(-5, -5),
                S(0, 0),     S(0, 0),     S(5, 5),     S(5, 5),     S(5, 5),     S(5, 5),     S(0, 0),     S(-5, -5),
                S(-10, -10), S(5, 5),     S(5, 5),     S(5, 5),     S(5, 5),     S(5, 5),     S(0, 0),     S(-10, -10),
                S(-10, -10), S(0, 0),     S(5, 5),     S(0, 0),     S(0, 0),     S(0, 0),     S(0, 0),     S(-10, -10),
                S(-20, -20), S(-10, -10), S(-10, -10), S(-5, -5),   S(-5, -5),   S(-10, -10), S(-10, -10), S(-20, -20)
            }, {    // KING
                S(-30, -50), S(-40, -40), S(-40, -30), S(-50, -20), S(-50, -20), S(-40, -30), S(-40, -40), S(-30, -50),
                S(-30, -30), S(-40, -20), S(-40, -10), S(-50, 0),   S(-50, 0),   S(-40, -10), S(-40, -20), S(-30, -30),
                S(-30, -30), S(-40, -10), S(-40, 20),  S(-50, 30),  S(-50, 30),  S(-40, 20),  S(-40, -10), S(-30, -30),
                S(-30, -30), S(-40, -10), S(-40, 30),  S(-50, 40),  S(-50, 40),  S(-40, 30),  S(-40, -10), S(-30, -30),
                S(-20, -30), S(-30, -10), S(-30, 30),  S(-40, 40),  S(-40, 40),  S(-30, 30),  S(-30, -10), S(-20, -30),
                S(-10, -30), S(-20, -10), S(-20, 20),  S(-20, 30),  S(-20, 30),  S(-20, 20),  S(-20, -10), S(-10, -30),
                S(20, -30),  S(20, -30),  S(0, 0),     S(0, 0),     S(0, 0),     S(0, 0),     S(20, -30),  S(20, -30),
                S(20, -50),  S(30, -30),  S(10, -30),  S(0, -30),   S(0, -30),   S(10, -30),  S(30, -30),  S(20, -50)
            }
        };
        // clang-format on

        static i32 PSQT[6][64][2];

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

        i32 evaluateBoard(const board::Board& board) {
            const u8 wQ = board.getPieceCount(Piece::WHITE_QUEEN);
            const u8 bQ = board.getPieceCount(Piece::BLACK_QUEEN);

            const u8 wR = board.getPieceCount(Piece::WHITE_ROOK);
            const u8 bR = board.getPieceCount(Piece::BLACK_ROOK);

            const u8 wB = board.getPieceCount(Piece::WHITE_BISHOP);
            const u8 bB = board.getPieceCount(Piece::BLACK_BISHOP);

            const u8 wN = board.getPieceCount(Piece::WHITE_KNIGHT);
            const u8 bN = board.getPieceCount(Piece::BLACK_KNIGHT);

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

            bool is_end_game = false;

            if (wQ == 0 && bQ == 0)
                is_end_game = true;
            else if (wQ == 1 && bQ == 1 && wR == 0 && bR == 0)
            {
                if ((wN + wB) <= 1 && (bN + bB) <= 1)
                    is_end_game = true;
            }

            i32 eval = is_end_game ? eval_eg : eval_mg;

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
            eval *= stm;

            return eval;
        }

    }

}
