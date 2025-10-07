#include "position.h"
#include "core/utils.h"

namespace sagittar {

    namespace core {

        static const std::array<std::array<Key, 64>, 16> ZOBRIST_TABLE = []() {
            std::array<std::array<Key, 64>, 16> table;
            for (int p = 0; p < 16; p++)
            {
                for (int sq = A1; sq <= H8; sq++)
                {
                    table[p][sq] = prng();
                }
            }
            return table;
        }();

        static const std::array<Key, 16> ZOBRIST_CA = []() {
            std::array<Key, 16> list;
            for (int i = 0; i < 16; i++)
            {
                list[i] = prng();
            }
            return list;
        }();

        static const Key ZOBRIST_SIDE = []() { return prng(); }();

        static int constexpr ZOBRIST_EP_IDX = 0;

        // clang-format off
        static const uint8_t CASTLE_RIGHTS_MODIFIERS[64] = {
            13, 15, 15, 15, 12, 15, 15, 14,
            15, 15, 15, 15, 15, 15, 15, 15,
            15, 15, 15, 15, 15, 15, 15, 15,
            15, 15, 15, 15, 15, 15, 15, 15,
            15, 15, 15, 15, 15, 15, 15, 15,
            15, 15, 15, 15, 15, 15, 15, 15,
            15, 15, 15, 15, 15, 15, 15, 15,
            7,  15, 15, 15,  3, 15, 15, 11
        };
        // clang-format on

        void Position::reset() { *this = Position{}; }

        void Position::reset_key() {
            m_key = 0ULL;

            m_key ^= ZOBRIST_SIDE & (-static_cast<uint64_t>(m_black_to_play));
            m_key ^= ZOBRIST_CA[m_ca_rights];
            m_key ^=
              ZOBRIST_TABLE[ZOBRIST_EP_IDX][m_ep_target] & (-static_cast<uint64_t>(!!m_ep_target));
            for (int sq = A1; sq <= H8; sq++)
            {
                const Piece p = m_board[sq];
                m_key ^= ZOBRIST_TABLE[p][sq] & (-static_cast<uint64_t>(p != NO_PIECE));
            }
        }

        bool Position::set_fen(std::string fen) {
            reset();

            std::string        segment;
            std::istringstream ss(fen);

            // Board
            int rank = Rank::RANK_8;
            int file = File::FILE_A;
            ss >> segment;
            for (const char& ch : segment)
            {
                if (ch == '/')
                {
                    rank--;
                    file = File::FILE_A;
                }
                else if (isdigit(ch))
                {
                    const int empty_squares = ch - '0';
                    if (empty_squares < 1 || empty_squares > 8) [[unlikely]]
                    {
                        throw std::invalid_argument("Invalid FEN!");
                    }
                    file += empty_squares;
                }
                else if (isalpha(ch))
                {
                    const PieceType pt = (ch == 'P' || ch == 'p') ? PAWN
                                       : (ch == 'N' || ch == 'n') ? KNIGHT
                                       : (ch == 'B' || ch == 'b') ? BISHOP
                                       : (ch == 'R' || ch == 'r') ? ROOK
                                       : (ch == 'Q' || ch == 'q') ? QUEEN
                                       : (ch == 'K' || ch == 'k') ? KING
                                                                  : PIECE_TYPE_INVALID;
                    const Color     color =
                      static_cast<Color>((ch == 'p') || (ch == 'n') || (ch == 'b') || (ch == 'r')
                                         || (ch == 'q') || (ch == 'k'));
                    set_piece(pt, color, RF_TO_SQ(rank, file++));
                }
            }

            assert(m_bb_pieces[PIECE_TYPE_INVALID] == 0ULL);

            // Active color
            ss >> segment;
            m_black_to_play = (segment == "b");

            // Castling rights
            ss >> segment;
            if (segment == "-")
            {
                m_ca_rights = CastlingRights::NOCA;
            }
            else
            {
                if (segment.find('K') != std::string::npos)
                {
                    m_ca_rights |= CastlingRights::WKCA;
                }
                if (segment.find('Q') != std::string::npos)
                {
                    m_ca_rights |= CastlingRights::WQCA;
                }
                if (segment.find('k') != std::string::npos)
                {
                    m_ca_rights |= CastlingRights::BKCA;
                }
                if (segment.find('q') != std::string::npos)
                {
                    m_ca_rights |= CastlingRights::BQCA;
                }
            }

            // En passant target
            ss >> segment;
            if (segment == "-")
            {
                m_ep_target = static_cast<Square>(0);
            }
            else
            {
                m_ep_target = static_cast<Square>(segment[0] - 'a' + 8 * (segment[1] - '1'));
            }

            // Halfmove clock
            ss >> segment;
            if (segment.empty() || segment == "-")
            {
                m_half_moves = 0;
            }
            else
            {
                m_half_moves = std::stoi(segment);
            }

            // Fullmove number
            ss >> segment;
            if (segment.empty() || segment == "-")
            {
                m_full_moves = 1;
            }
            else
            {
                m_full_moves = std::stoi(segment);
            }

#ifdef DEBUG
            assert_valid();
#endif

            reset_key();

            const Color us   = stm();
            const Color them = COLOR_FLIP(us);

            // King square
            const BitBoard king_bb = pieces(us, KING);
            m_king_sq              = static_cast<Square>(__builtin_ctzll(king_bb));

            // Checkers
            m_checkers = 0ULL;

            // Pinned
            m_pinned = 0ULL;

            return is_valid();
        }

        inline Piece Position::piece_on(const Square sq) const { return m_board[sq]; }

        inline BitBoard Position::pieces(const Color c) const { return m_bb_colors[c]; }

        inline Square Position::king_sq() const { return m_king_sq; }

        inline BitBoard Position::checkers() const { return m_checkers; }

        inline BitBoard Position::pinned() const { return m_pinned; }

        inline Color Position::stm() const { return static_cast<Color>(m_black_to_play); }

        inline uint8_t Position::ca_rights() const { return m_ca_rights; }

        inline Square Position::ep_target() const { return m_ep_target; }

        inline Key Position::key() const { return m_key; }

        inline bool Position::is_valid() const {
            return ((m_bb_pieces[PAWN] & BITBOARD_MASK_RANK_1_AND_8) == 0ULL)
                && (POPCNT(m_bb_pieces[KING]) == 2);
        }

        inline bool Position::is_drawn(std::span<uint64_t> key_history) const { return false; }

        inline bool Position::is_in_check() const { return (m_checkers != 0ULL); }

        void Position::display() const {
            std::cout << "\n   |---|---|---|---|---|---|---|---|\n";
            for (int r = RANK_8; r >= RANK_1; r--)
            {
                std::cout << (int) r + 1 << "  |";
                for (int f = FILE_A; f <= FILE_H; f++)
                {
                    const Square sq = RF_TO_SQ(r, f);
                    const Piece  p  = m_board[sq];
                    std::cout << " " << (char) PIECES_STR[p] << " |";
                }
                std::cout << "\n   |---|---|---|---|---|---|---|---|\n";
            }
            std::cout << "\n     a | b | c | d | e | f | g | h\n" << std::endl;

            const char active_color = m_black_to_play ? 'b' : 'w';
            std::cout << "\nActive Color\t\t: " << (char) active_color << "\n";
            std::cout << "Castling Rights\t\t: ";
            if (m_ca_rights == NOCA)
            {
                std::cout << "-";
            }
            else
            {
                if (m_ca_rights & WKCA)
                {
                    std::cout << "K";
                }
                if (m_ca_rights & WQCA)
                {
                    std::cout << "Q";
                }
                if (m_ca_rights & BKCA)
                {
                    std::cout << "k";
                }
                if (m_ca_rights & BQCA)
                {
                    std::cout << "q";
                }
            }
            std::cout << "\n";
            std::cout << "En passant Target\t: ";
            if (m_ep_target)
            {
                std::cout << (char) SQ_TO_FILE(m_ep_target) + 'a'
                          << (int) SQ_TO_RANK(m_ep_target) + 1;
            }
            else
            {
                std::cout << "-";
            }
            std::cout << "\n";
            std::cout << "Halfmove Clock\t\t: " << (int) m_half_moves << "\n";
            std::cout << "Fullmove Number\t\t: " << (int) m_full_moves << "\n";
            std::cout << "Hash\t\t\t: " << (uint64_t) m_key << "\n";
            std::cout << "Checkers\t\t: " << (uint64_t) m_checkers << "\n" << std::endl;
        }

        inline void Position::set_piece(const PieceType pt, const Color c, const Square sq) {
            const BitBoard sq_bb = BB(sq);
            m_bb_pieces[pt] |= sq_bb;
            m_bb_colors[c] |= sq_bb;
            m_board[sq] = PIECE_CREATE(pt, c);
        }

#ifdef DEBUG
        void Position::assert_valid() const {
            assert(is_valid());

            const BitBoard empty = ~(pieces(WHITE) | pieces(BLACK));
            for (int sq = A1; sq <= H8; sq++)
            {
                const BitBoard sq_bb = BB(sq);
                const Piece    p     = piece_on(static_cast<Square>(sq));
                if (p == NO_PIECE)
                {
                    assert((empty & sq_bb) == sq_bb);
                }
                else
                {
                    const PieceType pt   = PIECE_TYPE_OF(p);
                    const Color     c    = PIECE_COLOR_OF(p);
                    const BitBoard  p_bb = pieces(c, pt);
                    assert((p_bb & sq_bb) == sq_bb);
                }
            }
        }
#endif

    }
}
