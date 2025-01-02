#include "eval.h"

namespace sagittar {

    namespace eval {

        i32 evaluateBoard(const board::Board& board) {
            const u8 wQ = board.getPieceCount(Piece::WHITE_QUEEN);
            const u8 bQ = board.getPieceCount(Piece::BLACK_QUEEN);

            const u8 wR = board.getPieceCount(Piece::WHITE_ROOK);
            const u8 bR = board.getPieceCount(Piece::BLACK_ROOK);

            const u8 wB = board.getPieceCount(Piece::WHITE_BISHOP);
            const u8 bB = board.getPieceCount(Piece::BLACK_BISHOP);

            const u8 wN = board.getPieceCount(Piece::WHITE_KNIGHT);
            const u8 bN = board.getPieceCount(Piece::BLACK_KNIGHT);

            const u8 wP = board.getPieceCount(Piece::WHITE_PAWN);
            const u8 bP = board.getPieceCount(Piece::BLACK_PAWN);

            i32 eval = (900 * (wQ - bQ)) + (500 * (wR - bR)) + (300 * (wB - bB)) + (300 * (wN - bN))
                     + (100 * (wP - bP));

            const i8 stm = 1 - (2 * board.getActiveColor());
            eval *= stm;

            return eval;
        }

    }

}
