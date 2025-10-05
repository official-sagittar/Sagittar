#include "board.h"

namespace sagittar {

    namespace core {

        static constexpr BitBoard BITBOARD_MASK_RANK_1_AND_8 = 0xFF000000000000FF;

        Board::Board() { reset(); }

        void Board::reset() {
            bb_pieces = {};
            bb_colors = {};
            pieces    = {};
        }

        void Board::set_piece(const PieceType pt, const Color c, const Square sq) {
            const BitBoard sq_bb = BB(sq);
            bb_pieces[pt] |= sq_bb;
            bb_colors[c] |= sq_bb;
            pieces[sq] = PIECE_CREATE(pt, c);
        }

        bool Board::is_valid() const {
            return ((bb_pieces[PAWN] & BITBOARD_MASK_RANK_1_AND_8) == 0ULL)
                && (POPCNT(bb_pieces[KING]) == 2);
        }

        void Board::display() const {
            std::cout << "\n   |---|---|---|---|---|---|---|---|\n";
            for (int r = RANK_8; r >= RANK_1; r--)
            {
                std::cout << (int) r + 1 << "  |";
                for (int f = FILE_A; f <= FILE_H; f++)
                {
                    const Square sq = RF_TO_SQ(r, f);
                    const Piece  p  = pieces.at(sq);
                    std::cout << " " << (char) PIECES_STR[p] << " |";
                }
                std::cout << "\n   |---|---|---|---|---|---|---|---|\n";
            }
            std::cout << "\n     a | b | c | d | e | f | g | h\n" << std::endl;
        }

#ifdef DEBUG
        void Board::assert_valid() const {
            assert(is_valid());

            const BitBoard empty = ~(bb_colors[WHITE] | bb_colors[BLACK]);
            for (int sq = A1; sq <= H8; sq++)
            {
                const BitBoard sq_bb = BB(sq);
                const Piece    p     = pieces[sq];
                if (p == NO_PIECE)
                {
                    assert((empty & sq_bb) == sq_bb);
                }
                else
                {
                    const PieceType pt   = PIECE_TYPE_OF(p);
                    const Color     c    = PIECE_COLOR_OF(p);
                    const BitBoard  p_bb = (bb_pieces[pt] & bb_colors[c]);
                    assert((p_bb & sq_bb) == sq_bb);
                }
            }
        }
#endif

    }
}
