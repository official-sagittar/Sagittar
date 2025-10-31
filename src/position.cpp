#include "position.h"

#include "movegen.h"
#include "utils.h"

namespace sagittar {

    namespace core {

        // Little-Endian Rank-File Mapping
        // clang-format off
        static const u8 CASTLE_RIGHTS_MODIFIERS[64] = {
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

        static u64 ZOBRIST_TABLE[15][64];
        static u64 ZOBRIST_CA[16];
        static u64 ZOBRIST_SIDE;

        static constexpr u8 bitboardColorSlot(const Piece p) { return (7 + pieceColorOf(p)); }

        void Position::initialize() {

            for (u8 p = Piece::NO_PIECE; p <= Piece::BLACK_KING; p++)
            {
                for (u8 sq = Square::A1; sq <= Square::H8; sq++)
                {
                    ZOBRIST_TABLE[p][sq] = utils::prng();
                }
            }

            for (u8 i = 0; i < 16; i++)
            {
                ZOBRIST_CA[i] = utils::prng();
            }

            ZOBRIST_SIDE = utils::prng();

            movegen::initialize();
        }

        Position::Position() :
            m_bitboards({}),
            m_board({}),
            m_checkers(0ULL),
            m_stm(Color::WHITE),
            m_ca_rights(0),
            m_ep_target(Square::NO_SQ),
            m_halfmoves(0),
            m_fullmoves(0),
            m_ply_count(0),
            m_key(0ULL) {
            m_bitboards[Piece::NO_PIECE] = 0xFFFFFFFFFFFFFFFF;
        }

        void Position::reset() { *this = Position{}; }

        void Position::resetHash() {
            m_key = 0ULL;

            if (m_stm == Color::WHITE)
            {
                m_key ^= ZOBRIST_SIDE;
            }

            m_key ^= ZOBRIST_CA[m_ca_rights];

            for (u8 sq = Square::A1; sq <= Square::H8; sq++)
            {
                const Piece p = m_board[sq];

                if (p == Piece::NO_PIECE)
                {
                    continue;
                }

                m_key ^= ZOBRIST_TABLE[p][sq];
            }
        }

        void Position::setFen(std::string fen, const bool full) {
            std::string        segment;
            std::istringstream ss(fen);

            // Reset pos
            reset();

            // Parse Piece placement data
            u8 rank = Rank::RANK_8;
            u8 file = File::FILE_A;
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
                    const u8 empty_squares = ch - '0';
                    if (empty_squares < 1 || empty_squares > 8) [[unlikely]]
                    {
                        throw std::invalid_argument("Invalid FEN!");
                    }
                    file += empty_squares;
                }
                else if (isalpha(ch))
                {
                    std::size_t piece_id = PIECES_STR.find(ch);
                    if (piece_id == std::string::npos) [[unlikely]]
                    {
                        throw std::invalid_argument("Invalid FEN!");
                    }
                    setPiece(static_cast<Piece>(piece_id), rf2sq(rank, file++));
                }
            }

            // Parse Active color
            ss >> segment;
            if (segment == "w")
            {
                m_stm = Color::WHITE;
            }
            else if (segment == "b")
            {
                m_stm = Color::BLACK;
            }
            else [[unlikely]]
            {
                throw std::invalid_argument("Invalid FEN!");
            }

            // Parse Castling availability
            ss >> segment;
            if (segment == "-")
            {
                m_ca_rights = core::CastleFlag::NOCA;
            }
            else
            {
                if (segment.find('K') != std::string::npos)
                {
                    m_ca_rights |= core::CastleFlag::WKCA;
                }
                if (segment.find('Q') != std::string::npos)
                {
                    m_ca_rights |= core::CastleFlag::WQCA;
                }
                if (segment.find('k') != std::string::npos)
                {
                    m_ca_rights |= core::CastleFlag::BKCA;
                }
                if (segment.find('q') != std::string::npos)
                {
                    m_ca_rights |= core::CastleFlag::BQCA;
                }
            }

            // Parse En passant target square
            ss >> segment;
            if (segment == "-")
            {
                m_ep_target = Square::NO_SQ;
            }
            else
            {
                const Rank rank = static_cast<Rank>((segment[1] - '0') - 1);
                if (m_stm == Color::WHITE)
                {
                    if (rank != Rank::RANK_6) [[unlikely]]
                    {
                        throw std::invalid_argument("Invalid FEN!");
                    }
                }
                else
                {
                    if (rank != Rank::RANK_3) [[unlikely]]
                    {
                        throw std::invalid_argument("Invalid FEN!");
                    }
                }
                const File file = static_cast<File>(segment[0] - 'a');
                m_ep_target     = rf2sq(rank, file);
            }

            // Parse Halfmove clock
            ss >> segment;
            if (!full || segment.empty() || segment == "-")
            {
                m_halfmoves = 0;
            }
            else
            {
                m_halfmoves = std::stoi(segment);
            }

            // Parse Fullmove number
            ss >> segment;
            if (!full || segment.empty() || segment == "-")
            {
                m_fullmoves = 1;
            }
            else
            {
                m_fullmoves = std::stoi(segment);
            }

            // Set checkers
            const Piece    king = pieceCreate(PieceType::KING, m_stm);
            core::BitBoard bb   = m_bitboards[king];
            const Square   sq   = static_cast<Square>(utils::bitScanForward(&bb));
            m_checkers          = movegen::getSquareAttackers(*this, sq, colorFlip(m_stm));

            // Reset Hash
            resetHash();
        }

        std::string Position::toFen() const {
            std::ostringstream oss;

            // Piece placement data
            for (i8 rank = Rank::RANK_8; rank >= Rank::RANK_1; rank--)
            {
                u8 empty = 0;
                for (u8 file = File::FILE_A; file <= File::FILE_H; file++)
                {
                    const Square sq    = rf2sq(rank, file);
                    const Piece  piece = m_board[sq];
                    if (piece != Piece::NO_PIECE)
                    {
                        if (empty > 0)
                        {
                            oss << (int) empty;
                            empty = 0;
                        }
                        oss << PIECES_STR[piece];
                    }
                    else
                    {
                        empty++;
                    }
                }
                if (empty > 0)
                {
                    oss << (int) empty;
                }
                if (rank != Rank::RANK_1)
                {
                    oss << "/";
                }
            }
            oss << " ";

            // Active color
            if (m_stm == Color::WHITE)
            {
                oss << "w";
            }
            else
            {
                oss << "b";
            }
            oss << " ";

            // Castling availability
            if (m_ca_rights == core::CastleFlag::NOCA)
            {
                oss << "-";
            }
            else
            {
                if (m_ca_rights & core::CastleFlag::WKCA)
                {
                    oss << "K";
                }
                if (m_ca_rights & core::CastleFlag::WQCA)
                {
                    oss << "Q";
                }
                if (m_ca_rights & core::CastleFlag::BKCA)
                {
                    oss << "k";
                }
                if (m_ca_rights & core::CastleFlag::BQCA)
                {
                    oss << "q";
                }
            }
            oss << " ";

            // En passant target
            if (m_ep_target == Square::NO_SQ)
            {
                oss << "-";
            }
            else
            {
                const File file = sq2file(m_ep_target);
                const Rank rank = sq2rank(m_ep_target);
                oss << FILE_STR[file] << (int) rank + 1;
            }
            oss << " ";

            // Halfmove clock
            oss << (int) m_halfmoves << " ";

            // Fullmove number
            oss << (int) m_fullmoves << " ";

            return oss.str();
        }

        void Position::setPiece(const Piece piece, const Square square) {
#ifdef DEBUG
            assert(piece != Piece::NO_PIECE);
            assert(square != Square::NO_SQ);
#endif
            const BitBoard bit          = 1ULL << square;
            const BitBoard bit_inverted = ~(bit);
            m_bitboards[piece] |= bit;
            m_bitboards[Piece::NO_PIECE] &= bit_inverted;
            m_bitboards[bitboardColorSlot(piece)] |= bit;
            m_board[square] = piece;
            m_key ^= ZOBRIST_TABLE[piece][square];
        }

        void Position::clearPiece(const Piece piece, const Square square) {
#ifdef DEBUG
            assert(piece != Piece::NO_PIECE);
            assert(square != Square::NO_SQ);
#endif
            const BitBoard bit          = 1ULL << square;
            const BitBoard bit_inverted = ~(bit);
            m_bitboards[piece] &= bit_inverted;
            m_bitboards[Piece::NO_PIECE] |= bit;
            m_bitboards[bitboardColorSlot(piece)] &= bit_inverted;
            m_board[square] = Piece::NO_PIECE;
            m_key ^= ZOBRIST_TABLE[piece][square];
        }

        void Position::movePiece(const Piece  piece,
                                 const Square from,
                                 const Square to,
                                 const bool   is_capture,
                                 const bool   is_promotion,
                                 const Piece  promoted) {
#ifdef DEBUG
            assert(piece != Piece::NO_PIECE);
            assert(from != Square::NO_SQ);
            assert(to != Square::NO_SQ);
#endif

            clearPiece(piece, from);

            if (is_capture)
            {
                const Piece captured = m_board[to];
#ifdef DEBUG
                assert(captured != Piece::NO_PIECE);
                assert(pieceColorOf(piece) == colorFlip(pieceColorOf(captured)));
#endif
                clearPiece(captured, to);
            }

            if (is_promotion)
            {
#ifdef DEBUG
                assert(pieceTypeOf(piece) == PieceType::PAWN);
                assert(sq2rank(from) == promotionRankSrcOf(pieceColorOf(piece)));
                assert(sq2rank(to) == promotionRankDestOf(pieceColorOf(piece)));
                assert(promoted != Piece::NO_PIECE);
                assert(pieceTypeOf(promoted) != PieceType::PAWN);
                assert(pieceTypeOf(promoted) != PieceType::KING);
                assert(pieceColorOf(piece) == pieceColorOf(promoted));
                assert(m_board[to] == Piece::NO_PIECE);
#endif
                setPiece(promoted, to);
            }
            else
            {
#ifdef DEBUG
                assert(m_board[to] == Piece::NO_PIECE);
#endif
                setPiece(piece, to);
            }
        }

        DoMoveResult Position::doMoveComplete() {
            const Color them = colorFlip(m_stm);

            // Check if move does not leave our King in check and pos is valid
            Piece          king          = pieceCreate(PieceType::KING, m_stm);
            core::BitBoard bb            = m_bitboards[king];
            Square         sq            = static_cast<Square>(utils::bitScanForward(&bb));
            const BitBoard checkers_us   = movegen::getSquareAttackers(*this, sq, them);
            const bool     is_valid_move = (checkers_us == 0ULL) && isValid();

            // Set checkers
            king       = pieceCreate(PieceType::KING, them);
            bb         = m_bitboards[king];
            sq         = static_cast<Square>(utils::bitScanForward(&bb));
            m_checkers = movegen::getSquareAttackers(*this, sq, m_stm);

            // Switch sides
            m_stm = them;
            m_key ^= ZOBRIST_SIDE;
#ifdef DEBUG
            const u64 currhash = m_key;
            resetHash();
            assert(currhash == m_key);
#endif
            return is_valid_move ? DoMoveResult::LEGAL : DoMoveResult::ILLEGAL;
        }

        [[nodiscard]] DoMoveResult Position::doMove(const move::Move& move) noexcept {
            const Square         from  = move.from();
            const Square         to    = move.to();
            const move::MoveFlag flag  = move.flag();
            const Piece          piece = m_board[from];

            if (pieceColorOf(piece) == colorFlip(m_stm)) [[unlikely]]
            {
                return DoMoveResult::INVALID;
            }

            m_ep_target = Square::NO_SQ;
            m_halfmoves++;
            if (m_stm == Color::BLACK)
            {
                m_fullmoves++;
            }
            m_ply_count++;

            if (flag == move::MoveFlag::MOVE_QUIET_PAWN_DBL_PUSH)
            {
                movePiece(piece, from, to);
                const i8 stm = 1 - (2 * m_stm);  // WHITE = 1; BLACK = -1
                m_ep_target  = static_cast<Square>(from + (8 * stm));
                m_halfmoves  = 0;
                return doMoveComplete();
            }
            else if (flag == move::MoveFlag::MOVE_CAPTURE_EP)
            {
                movePiece(piece, from, to);
                const i8     stm         = -1 + (2 * m_stm);  // WHITE = -1; BLACK = 1
                const Square captured_sq = static_cast<Square>(to + (8 * stm));
                const Piece  captured    = pieceCreate(PieceType::PAWN, colorFlip(m_stm));
                clearPiece(captured, captured_sq);
                m_halfmoves = 0;
                return doMoveComplete();
            }
            else if (flag == move::MoveFlag::MOVE_CASTLE_KING_SIDE)
            {
                movePiece(piece, from, to);
                const Piece rook = pieceCreate(PieceType::ROOK, m_stm);
                movePiece(rook, static_cast<Square>(to + 1), static_cast<Square>(to - 1));
                m_key ^= ZOBRIST_CA[m_ca_rights];
                m_ca_rights &= CASTLE_RIGHTS_MODIFIERS[from];
                m_key ^= ZOBRIST_CA[m_ca_rights];
                return doMoveComplete();
            }
            else if (flag == move::MoveFlag::MOVE_CASTLE_QUEEN_SIDE)
            {
                movePiece(piece, from, to);
                const Piece rook = pieceCreate(PieceType::ROOK, m_stm);
                movePiece(rook, static_cast<Square>(to - 2), static_cast<Square>(to + 1));
                m_key ^= ZOBRIST_CA[m_ca_rights];
                m_ca_rights &= CASTLE_RIGHTS_MODIFIERS[from];
                m_key ^= ZOBRIST_CA[m_ca_rights];
                return doMoveComplete();
            }

            Piece promoted;

            switch (flag)
            {
                case move::MoveFlag::MOVE_PROMOTION_KNIGHT :
                case move::MoveFlag::MOVE_CAPTURE_PROMOTION_KNIGHT :
                    promoted = pieceCreate(PieceType::KNIGHT, m_stm);
                    break;
                case move::MoveFlag::MOVE_PROMOTION_BISHOP :
                case move::MoveFlag::MOVE_CAPTURE_PROMOTION_BISHOP :
                    promoted = pieceCreate(PieceType::BISHOP, m_stm);
                    break;
                case move::MoveFlag::MOVE_PROMOTION_ROOK :
                case move::MoveFlag::MOVE_CAPTURE_PROMOTION_ROOK :
                    promoted = pieceCreate(PieceType::ROOK, m_stm);
                    break;
                case move::MoveFlag::MOVE_PROMOTION_QUEEN :
                case move::MoveFlag::MOVE_CAPTURE_PROMOTION_QUEEN :
                    promoted = pieceCreate(PieceType::QUEEN, m_stm);
                    break;
                default :
                    promoted = Piece::NO_PIECE;
            }

            movePiece(piece, from, to, move.isCapture(), move.isPromotion(), promoted);

            if ((pieceTypeOf(piece) == PieceType::PAWN) || move.isCapture())
            {
                m_halfmoves = 0;
            }

            m_key ^= ZOBRIST_CA[m_ca_rights];
            m_ca_rights &= (CASTLE_RIGHTS_MODIFIERS[from] & CASTLE_RIGHTS_MODIFIERS[to]);
            m_key ^= ZOBRIST_CA[m_ca_rights];

            return doMoveComplete();
        }

        [[nodiscard]] DoMoveResult Position::doMove(const std::string& move_str) noexcept {
            const std::size_t len = move_str.length();
            if (len < 4 || len > 5)
            {
                return DoMoveResult::INVALID;
            }

            const u8 from_file = move_str[0] - 'a';
            const u8 from_rank = (move_str[1] - '0') - 1;
            const u8 to_file   = move_str[2] - 'a';
            const u8 to_rank   = (move_str[3] - '0') - 1;

            if (from_file < File::FILE_A || from_file > File::FILE_H)
            {
                return DoMoveResult::INVALID;
            }
            if (to_file < File::FILE_A || to_file > File::FILE_H)
            {
                return DoMoveResult::INVALID;
            }
            if (from_rank < Rank::RANK_1 || from_rank > Rank::RANK_8)
            {
                return DoMoveResult::INVALID;
            }
            if (to_rank < Rank::RANK_1 || to_rank > Rank::RANK_8)
            {
                return DoMoveResult::INVALID;
            }

            const bool is_promotion = (len == 5);

            if (is_promotion
                && (from_rank != promotionRankSrcOf(m_stm)
                    || to_rank != promotionRankDestOf(m_stm)))
            {
                return DoMoveResult::INVALID;
            }

            const Square from       = rf2sq(from_rank, from_file);
            const Square to         = rf2sq(to_rank, to_file);
            const Piece  piece      = m_board[from];
            const bool   is_capture = (m_board[to] != Piece::NO_PIECE);

            move::MoveFlag flag;

            if (piece == Piece::NO_PIECE)
            {
                return DoMoveResult::INVALID;
            }

            if (pieceTypeOf(piece) == PieceType::PAWN)
            {
                if (!is_promotion)
                {
                    if (!is_capture)
                    {
                        if (m_stm == Color::WHITE && from_rank == Rank::RANK_2
                            && to_rank == Rank::RANK_4)
                        {
                            flag = move::MoveFlag::MOVE_QUIET_PAWN_DBL_PUSH;
                        }
                        else if (m_stm == Color::BLACK && from_rank == Rank::RANK_7
                                 && to_rank == Rank::RANK_5)
                        {
                            flag = move::MoveFlag::MOVE_QUIET_PAWN_DBL_PUSH;
                        }
                        else if (to == m_ep_target)
                        {
                            flag = move::MoveFlag::MOVE_CAPTURE_EP;
                        }
                        else
                        {
                            flag = move::MoveFlag::MOVE_QUIET;
                        }
                    }
                    else
                    {
                        flag = move::MoveFlag::MOVE_CAPTURE;
                    }
                }
                else
                {
                    switch (move_str[4])
                    {
                        case 'n' :
                            flag = is_capture ? move::MoveFlag::MOVE_CAPTURE_PROMOTION_KNIGHT
                                              : move::MoveFlag::MOVE_PROMOTION_KNIGHT;
                            break;
                        case 'b' :
                            flag = is_capture ? move::MoveFlag::MOVE_CAPTURE_PROMOTION_BISHOP
                                              : move::MoveFlag::MOVE_PROMOTION_BISHOP;
                            break;
                        case 'r' :
                            flag = is_capture ? move::MoveFlag::MOVE_CAPTURE_PROMOTION_ROOK
                                              : move::MoveFlag::MOVE_PROMOTION_ROOK;
                            break;
                        case 'q' :
                            flag = is_capture ? move::MoveFlag::MOVE_CAPTURE_PROMOTION_QUEEN
                                              : move::MoveFlag::MOVE_PROMOTION_QUEEN;
                            break;
                        default :
                            return DoMoveResult::INVALID;
                    }
                }
            }
            else if (pieceTypeOf(m_board[from]) == PieceType::KING)
            {
                if (m_stm == Color::WHITE)
                {
                    if (from == Square::E1 && to == Square::G1 && (m_ca_rights & CastleFlag::WKCA))
                    {
                        flag = move::MoveFlag::MOVE_CASTLE_KING_SIDE;
                    }
                    else if (from == Square::E1 && to == Square::C1
                             && (m_ca_rights & CastleFlag::WQCA))
                    {
                        flag = move::MoveFlag::MOVE_CASTLE_QUEEN_SIDE;
                    }
                    else
                    {
                        flag =
                          is_capture ? move::MoveFlag::MOVE_CAPTURE : move::MoveFlag::MOVE_QUIET;
                    }
                }
                else
                {
                    if (from == Square::E8 && to == Square::G8 && (m_ca_rights & CastleFlag::BKCA))
                    {
                        flag = move::MoveFlag::MOVE_CASTLE_KING_SIDE;
                    }
                    else if (from == Square::E8 && to == Square::C8
                             && (m_ca_rights & CastleFlag::BQCA))
                    {
                        flag = move::MoveFlag::MOVE_CASTLE_QUEEN_SIDE;
                    }
                    else
                    {
                        flag =
                          is_capture ? move::MoveFlag::MOVE_CAPTURE : move::MoveFlag::MOVE_QUIET;
                    }
                }
            }
            else
            {
                flag = is_capture ? move::MoveFlag::MOVE_CAPTURE : move::MoveFlag::MOVE_QUIET;
            }

            const move::Move move(from, to, flag);

            return doMove(move);
        }

        void Position::doNullMove() {
            m_checkers  = 0ULL;
            m_ep_target = Square::NO_SQ;
            m_stm       = colorFlip(m_stm);
            m_key ^= ZOBRIST_SIDE;
        }

        BitBoard Position::pieces(const Color c) const { return m_bitboards[(7 + c)]; }

        BitBoard Position::pieces(const Color c, const PieceType pt) const {
            return m_bitboards[pieceCreate(pt, c)];
        }

        BitBoard Position::occupied() const { return ~m_bitboards[Piece::NO_PIECE]; }

        BitBoard Position::empty() const { return m_bitboards[Piece::NO_PIECE]; }

        Piece Position::pieceOn(const Square square) const { return m_board[square]; }

        u8 Position::pieceCount(const Piece piece) const {
            return utils::bitCount1s(m_bitboards[piece]);
        }

        Color Position::stm() const { return m_stm; }

        u8 Position::caRights() const { return m_ca_rights; }

        Square Position::epTarget() const { return m_ep_target; }

        u8 Position::halfmoves() const { return m_halfmoves; }

        u8 Position::fullmoves() const { return m_fullmoves; }

        u64 Position::key() const { return m_key; }

        bool Position::isValid() const {
            const bool check = (pieceCount(Piece::WHITE_KING) == 1)
                            && (pieceCount(Piece::BLACK_KING) == 1)
                            && (!(m_bitboards[Piece::WHITE_PAWN] & MASK_RANK_1))
                            && (!(m_bitboards[Piece::WHITE_PAWN] & MASK_RANK_8))
                            && (!(m_bitboards[Piece::BLACK_PAWN] & MASK_RANK_1))
                            && (!(m_bitboards[Piece::BLACK_PAWN] & MASK_RANK_8));
            return check;
        }

        bool Position::isInCheck() const { return (m_checkers != 0ULL); }

        bool Position::isDrawn(std::span<u64> key_history) const {
#ifdef DEBUG
            assert(key_history.size() == static_cast<size_t>(m_ply_count));
#endif
            for (i32 i = std::max(m_ply_count - m_halfmoves, 0); i < m_ply_count - 1; ++i)
            {
                if (m_key == key_history[i])
                {
                    return true;
                }
            }
            return false;
        }

        void Position::display() const {
            std::ostringstream ss;

            for (i8 rank = Rank::RANK_8; rank >= Rank::RANK_1; rank--)
            {
                for (u8 file = File::FILE_A; file <= File::FILE_H; file++)
                {
                    const Square sq = rf2sq(rank, file);
                    ss << (char) PIECES_STR[this->m_board[sq]] << " ";
                }
                ss << "\n";
            }

            ss << "\n" << (char) COLORS_STR[m_stm];
            ss << " " << (int) m_ca_rights;
            ss << " " << (int) m_ep_target;
            ss << " " << (int) m_halfmoves;
            ss << " " << (int) m_fullmoves;
            ss << " " << (int) m_ply_count;
            ss << " " << (unsigned long long) m_key << "\n";

            std::cout << ss.str() << toFen() << std::endl;
        }

        bool Position::operator==(Position const& rhs) const { return m_key == rhs.key(); }

    }

}
