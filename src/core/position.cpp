#include "position.h"
#include "commons/utils.h"
#include "core/movegen.h"

namespace sagittar {

    // Little-Endian Rank-File Mapping
    // clang-format off
    static const U8 k_CASTLE_RIGHTS_MODIFIERS[64] = {
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

    static std::array<std::array<U64, 64>, 15> g_zobrist_table;  // [Piece][Square]
    static std::array<U64, 16>                 g_zobrist_ca;
    static U64                                 g_zobrist_side;
    static int constexpr k_ZOBRIST_EP_IDX = 14;

    void Position::initialize() {

        for (U8 p = Piece::WHITE_PAWN; p <= Piece::NO_PIECE; p++)
        {
            for (U8 sq = Square::A1; sq <= Square::H8; sq++)
            {
                g_zobrist_table[p][sq] = utils::prng();
            }
        }

        for (U8 i = 0; i < 16; i++)
        {
            g_zobrist_ca[i] = utils::prng();
        }

        g_zobrist_side = utils::prng();

        movegenInitialize();
    }

    Position::Position() :
        m_bb_pieces({}),
        m_bb_colors({}),
        m_checkers(0ULL),
        m_king_sq(Square::NO_SQ),
        m_stm(Color::WHITE),
        m_ca_rights(0),
        m_ep_target(Square::NO_SQ),
        m_halfmoves(0),
        m_fullmoves(0),
        m_ply_count(0),
        m_key(0ULL),
        m_pawn_key(0ULL) {
        m_board.fill(Piece::NO_PIECE);
    }

    void Position::reset() { *this = Position{}; }

    void Position::resetHash() {
        m_key      = 0ULL;
        m_pawn_key = 0ULL;

        // Side to move
        if (m_stm == Color::WHITE)
        {
            m_key ^= g_zobrist_side;
        }

        // Castling Rights
        m_key ^= g_zobrist_ca[m_ca_rights];

        // EP Target
        if (m_ep_target != Square::NO_SQ)
        {
            m_key ^= g_zobrist_table[k_ZOBRIST_EP_IDX][m_ep_target];
        }

        // Piece placement
        for (U8 sq = Square::A1; sq <= Square::H8; sq++)
        {
            const Piece p = m_board[sq];

            if (p == Piece::NO_PIECE)
            {
                continue;
            }

            m_key ^= g_zobrist_table[p][sq];

            if (pieceTypeOf(p) == PieceType::PAWN)
            {
                m_pawn_key ^= g_zobrist_table[p][sq];
            }
        }
    }

    void Position::setFen(std::string fen, const bool full) {
        std::string        segment;
        std::istringstream ss(fen);

        // Reset pos
        reset();

        // Parse Piece placement data
        U8 rank = Rank::RANK_8;
        U8 file = File::FILE_A;
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
                const U8 empty_squares = ch - '0';
                if (empty_squares < 1 || empty_squares > 8) [[unlikely]]
                {
                    throw std::invalid_argument("Invalid FEN!");
                }
                file += empty_squares;
            }
            else if (isalpha(ch))
            {
                const PieceType pt = (ch == 'P' || ch == 'p') ? PieceType::PAWN
                                   : (ch == 'N' || ch == 'n') ? PieceType::KNIGHT
                                   : (ch == 'B' || ch == 'b') ? PieceType::BISHOP
                                   : (ch == 'R' || ch == 'r') ? PieceType::ROOK
                                   : (ch == 'Q' || ch == 'q') ? PieceType::QUEEN
                                   : (ch == 'K' || ch == 'k') ? PieceType::KING
                                                              : PieceType::PIECE_TYPE_INVALID;
                if (pt == PieceType::PIECE_TYPE_INVALID) [[unlikely]]
                {
                    throw std::invalid_argument("Invalid FEN!");
                }
                const Color    c = static_cast<Color>((ch == 'p') || (ch == 'n') || (ch == 'b')
                                                      || (ch == 'r') || (ch == 'q') || (ch == 'k'));
                const Square   sq    = rf2sq(rank, file);
                const BitBoard sq_bb = bb(sq);
                m_bb_pieces[pt] |= sq_bb;
                m_bb_colors[c] |= sq_bb;
                m_board[sq] = pieceCreate(pt, c);
                ++file;
            }
        }

        if (!isValid()) [[unlikely]]
        {
            throw std::invalid_argument("Invalid FEN!");
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
            m_ca_rights = CastleFlag::NOCA;
        }
        else
        {
            if (segment.find('K') != std::string::npos)
            {
                m_ca_rights |= CastleFlag::WKCA;
            }
            if (segment.find('Q') != std::string::npos)
            {
                m_ca_rights |= CastleFlag::WQCA;
            }
            if (segment.find('k') != std::string::npos)
            {
                m_ca_rights |= CastleFlag::BKCA;
            }
            if (segment.find('q') != std::string::npos)
            {
                m_ca_rights |= CastleFlag::BQCA;
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

        // Parse Halfmove Clock
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

        // Set King Square & checkers
        BitBoard king_bb = m_bb_pieces[PieceType::KING] & m_bb_colors[m_stm];
        m_king_sq        = static_cast<Square>(king_bb.popLsb());
        m_checkers       = squareAttackers(*this, m_king_sq, colorFlip(m_stm));

        // Reset Hash
        resetHash();
    }

    std::string Position::toFen() const {
        std::ostringstream oss;

        // Piece placement data
        for (I8 rank = Rank::RANK_8; rank >= Rank::RANK_1; rank--)
        {
            U8 empty = 0;
            for (U8 file = File::FILE_A; file <= File::FILE_H; file++)
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
                    oss << k_PIECES_STR[piece];
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
        if (m_ca_rights == CastleFlag::NOCA)
        {
            oss << "-";
        }
        else
        {
            if (m_ca_rights & CastleFlag::WKCA)
            {
                oss << "K";
            }
            if (m_ca_rights & CastleFlag::WQCA)
            {
                oss << "Q";
            }
            if (m_ca_rights & CastleFlag::BKCA)
            {
                oss << "k";
            }
            if (m_ca_rights & CastleFlag::BQCA)
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
            oss << k_FILE_STR[file] << (int) rank + 1;
        }
        oss << " ";

        // Halfmove Clock
        oss << (int) m_halfmoves << " ";

        // Fullmove number
        oss << (int) m_fullmoves << " ";

        return oss.str();
    }

    template<Color US, MoveFlag F>
    bool Position::applyMove(const Move& move) noexcept {
        constexpr bool  is_capture   = moveIsCapture(F);
        constexpr bool  is_promotion = moveIsPromotion(F);
        constexpr Color them         = colorFlip(US);

        U64 key_local      = m_key;
        U64 pawn_key_local = m_pawn_key;

        key_local ^=
          (m_ep_target == Square::NO_SQ) ? 0ULL : g_zobrist_table[k_ZOBRIST_EP_IDX][m_ep_target];

        m_ep_target = Square::NO_SQ;
        ++m_halfmoves;
        m_fullmoves += (m_stm == Color::BLACK);
        ++m_ply_count;

        const Square from = move.from();
        const Square to   = move.to();

        const Piece     move_p  = m_board[from];
        const PieceType move_pt = pieceTypeOf(move_p);

        const Piece captured_p = m_board[to];

        const BitBoard move_mask_to = bb(to);
        const BitBoard move_mask    = bb(from) | move_mask_to;

        m_bb_pieces[move_pt] ^= move_mask;
        m_bb_colors[US] ^= move_mask;
        m_board[from] = Piece::NO_PIECE;
        m_board[to]   = move_p;

        key_local ^= g_zobrist_table[move_p][from];
        key_local ^= g_zobrist_table[move_p][to];
        if (move_pt == PieceType::PAWN)
        {
            pawn_key_local ^= g_zobrist_table[move_p][from];
            pawn_key_local ^= g_zobrist_table[move_p][to];
        }

        if constexpr ((F == MoveFlag::MOVE_CASTLE_KING_SIDE)
                      || (F == MoveFlag::MOVE_CASTLE_QUEEN_SIDE))
        {
            assert(move_pt == PieceType::KING);
            constexpr Piece rook = pieceCreate(PieceType::ROOK, US);
            constexpr Rank  rank = (US == Color::WHITE) ? Rank::RANK_1 : Rank::RANK_8;
            constexpr File  from_file =
              (F == MoveFlag::MOVE_CASTLE_KING_SIDE) ? File::FILE_H : File::FILE_A;
            constexpr File to_file =
              (F == MoveFlag::MOVE_CASTLE_KING_SIDE) ? File::FILE_F : File::FILE_D;
            constexpr Square   ca_r_from_sq   = rf2sq(rank, from_file);
            constexpr Square   ca_r_to_sq     = rf2sq(rank, to_file);
            constexpr BitBoard move_mask_ca_r = (bb(ca_r_from_sq) | bb(ca_r_to_sq));
            m_bb_pieces[PieceType::ROOK] ^= move_mask_ca_r;
            m_bb_colors[US] ^= move_mask_ca_r;
            m_board[ca_r_from_sq] = Piece::NO_PIECE;
            m_board[ca_r_to_sq]   = rook;
            key_local ^= g_zobrist_table[rook][ca_r_from_sq];
            key_local ^= g_zobrist_table[rook][ca_r_to_sq];
        }
        else if constexpr (F == MoveFlag::MOVE_QUIET_PAWN_DBL_PUSH)
        {
            assert(move_pt == PieceType::PAWN);
            constexpr I8 dir = (US == Color::WHITE) ? 8 : -8;
            m_ep_target      = static_cast<Square>(from + dir);
            key_local ^= g_zobrist_table[k_ZOBRIST_EP_IDX][m_ep_target];
        }
        else if constexpr (is_capture || is_promotion)
        {
            if constexpr (is_capture)
            {
                if constexpr (F == MoveFlag::MOVE_CAPTURE_EP)
                {
                    assert(move_pt == PieceType::PAWN);
                    assert(captured_p == Piece::NO_PIECE);
                    constexpr Piece ep_victim       = pieceCreate(PieceType::PAWN, them);
                    constexpr I8    dir             = (US == Color::WHITE) ? 8 : -8;
                    const Square    ep_victim_sq    = static_cast<Square>(to - dir);
                    const BitBoard  ep_victim_sq_bb = bb(ep_victim_sq);
                    m_bb_pieces[PieceType::PAWN] ^= ep_victim_sq_bb;
                    m_bb_colors[them] ^= ep_victim_sq_bb;
                    m_board[ep_victim_sq] = Piece::NO_PIECE;
                    key_local ^= g_zobrist_table[ep_victim][ep_victim_sq];
                    pawn_key_local ^= g_zobrist_table[ep_victim][ep_victim_sq];
                }
                else
                {
                    assert(captured_p != Piece::NO_PIECE);
                    const PieceType captured_pt = pieceTypeOf(captured_p);
                    m_bb_pieces[captured_pt] ^= move_mask_to;
                    m_bb_colors[them] ^= move_mask_to;
                    key_local ^= g_zobrist_table[captured_p][to];
                    if (captured_pt == PieceType::PAWN)
                    {
                        pawn_key_local ^= g_zobrist_table[captured_p][to];
                    }
                }
            }

            if constexpr (is_promotion)
            {
                assert(move_pt == PieceType::PAWN);
                constexpr PieceType promoted_pt =
                  ((F == MoveFlag::MOVE_PROMOTION_KNIGHT)
                   || (F == MoveFlag::MOVE_CAPTURE_PROMOTION_KNIGHT))
                    ? PieceType::KNIGHT
                  : ((F == MoveFlag::MOVE_PROMOTION_BISHOP)
                     || (F == MoveFlag::MOVE_CAPTURE_PROMOTION_BISHOP))
                    ? PieceType::BISHOP
                  : ((F == MoveFlag::MOVE_PROMOTION_ROOK)
                     || (F == MoveFlag::MOVE_CAPTURE_PROMOTION_ROOK))
                    ? PieceType::ROOK
                  : ((F == MoveFlag::MOVE_PROMOTION_QUEEN)
                     || (F == MoveFlag::MOVE_CAPTURE_PROMOTION_QUEEN))
                    ? PieceType::QUEEN
                    : PieceType::PIECE_TYPE_INVALID;
                assert((promoted_pt != PieceType::PAWN) || (promoted_pt != PieceType::KING)
                       || (promoted_pt != PieceType::PIECE_TYPE_INVALID));
                constexpr Piece promoted = pieceCreate(promoted_pt, US);
                m_bb_pieces[PieceType::PAWN] ^= move_mask_to;
                m_bb_pieces[promoted_pt] ^= move_mask_to;
                m_board[to] = promoted;
                key_local ^= g_zobrist_table[move_p][to];
                key_local ^= g_zobrist_table[promoted][to];
                pawn_key_local ^= g_zobrist_table[move_p][to];
            }
        }

        m_halfmoves *= !(is_capture || (move_pt == PieceType::PAWN));

        key_local ^= g_zobrist_ca[m_ca_rights];
        m_ca_rights &= k_CASTLE_RIGHTS_MODIFIERS[from];
        m_ca_rights &= k_CASTLE_RIGHTS_MODIFIERS[to];
        key_local ^= g_zobrist_ca[m_ca_rights];

        const BitBoard k_bb = m_bb_pieces[PieceType::KING];

        const BitBoard king_bb_us  = k_bb & m_bb_colors[US];
        const Square   king_sq_us  = static_cast<Square>(king_bb_us.lsb());
        const BitBoard checkers_us = squareAttackers(*this, king_sq_us, them);

        const bool is_valid_move = checkers_us.isEmpty();

        const BitBoard king_bb_them = k_bb & m_bb_colors[them];
        m_king_sq                   = static_cast<Square>(king_bb_them.lsb());
        m_checkers                  = squareAttackers(*this, m_king_sq, US);

        m_stm = colorFlip(m_stm);
        key_local ^= g_zobrist_side;

        m_key      = key_local;
        m_pawn_key = pawn_key_local;

#ifdef DEBUG
        const U64 curr_key      = m_key;
        const U64 curr_pawn_key = m_pawn_key;
        resetHash();
        assert(m_key == curr_key);
        assert(m_pawn_key == curr_pawn_key);
#endif

        return is_valid_move;
    }

    [[nodiscard]] bool Position::doMove(const Move& move) noexcept {
        const bool is_valid = (pieceColorOf(pieceOn(move.from())) == m_stm);
        return (m_stm == Color::WHITE)
               ? is_valid && (this->*k_APPLY_MOVE_DISPATCH_TABLE<Color::WHITE>[move.flag()])(move)
               : is_valid && (this->*k_APPLY_MOVE_DISPATCH_TABLE<Color::BLACK>[move.flag()])(move);
    }

    [[nodiscard]] bool Position::doMove(const std::string& move_str) noexcept {
        const std::size_t len = move_str.length();
        if (len < 4 || len > 5)
        {
            return false;
        }

        const U8 from_file = move_str[0] - 'a';
        const U8 from_rank = (move_str[1] - '0') - 1;
        const U8 to_file   = move_str[2] - 'a';
        const U8 to_rank   = (move_str[3] - '0') - 1;

        if (from_file < File::FILE_A || from_file > File::FILE_H)
        {
            return false;
        }
        if (to_file < File::FILE_A || to_file > File::FILE_H)
        {
            return false;
        }
        if (from_rank < Rank::RANK_1 || from_rank > Rank::RANK_8)
        {
            return false;
        }
        if (to_rank < Rank::RANK_1 || to_rank > Rank::RANK_8)
        {
            return false;
        }

        const bool is_promotion = (len == 5);

        if (is_promotion
            && (from_rank != promotionRankSrcOf(m_stm) || to_rank != promotionRankDestOf(m_stm)))
        {
            return false;
        }

        const Square from       = rf2sq(from_rank, from_file);
        const Square to         = rf2sq(to_rank, to_file);
        const Piece  piece      = m_board[from];
        const bool   is_capture = (m_board[to] != Piece::NO_PIECE);

        MoveFlag flag;

        if (piece == Piece::NO_PIECE)
        {
            return false;
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
                        flag = MoveFlag::MOVE_QUIET_PAWN_DBL_PUSH;
                    }
                    else if (m_stm == Color::BLACK && from_rank == Rank::RANK_7
                             && to_rank == Rank::RANK_5)
                    {
                        flag = MoveFlag::MOVE_QUIET_PAWN_DBL_PUSH;
                    }
                    else if (to == m_ep_target)
                    {
                        flag = MoveFlag::MOVE_CAPTURE_EP;
                    }
                    else
                    {
                        flag = MoveFlag::MOVE_QUIET;
                    }
                }
                else
                {
                    flag = MoveFlag::MOVE_CAPTURE;
                }
            }
            else
            {
                switch (move_str[4])
                {
                    case 'n' :
                        flag = is_capture ? MoveFlag::MOVE_CAPTURE_PROMOTION_KNIGHT
                                          : MoveFlag::MOVE_PROMOTION_KNIGHT;
                        break;
                    case 'b' :
                        flag = is_capture ? MoveFlag::MOVE_CAPTURE_PROMOTION_BISHOP
                                          : MoveFlag::MOVE_PROMOTION_BISHOP;
                        break;
                    case 'r' :
                        flag = is_capture ? MoveFlag::MOVE_CAPTURE_PROMOTION_ROOK
                                          : MoveFlag::MOVE_PROMOTION_ROOK;
                        break;
                    case 'q' :
                        flag = is_capture ? MoveFlag::MOVE_CAPTURE_PROMOTION_QUEEN
                                          : MoveFlag::MOVE_PROMOTION_QUEEN;
                        break;
                    default :
                        return false;
                }
            }
        }
        else if (pieceTypeOf(m_board[from]) == PieceType::KING)
        {
            if (m_stm == Color::WHITE)
            {
                if (from == Square::E1 && to == Square::G1 && (m_ca_rights & CastleFlag::WKCA))
                {
                    flag = MoveFlag::MOVE_CASTLE_KING_SIDE;
                }
                else if (from == Square::E1 && to == Square::C1 && (m_ca_rights & CastleFlag::WQCA))
                {
                    flag = MoveFlag::MOVE_CASTLE_QUEEN_SIDE;
                }
                else
                {
                    flag = is_capture ? MoveFlag::MOVE_CAPTURE : MoveFlag::MOVE_QUIET;
                }
            }
            else
            {
                if (from == Square::E8 && to == Square::G8 && (m_ca_rights & CastleFlag::BKCA))
                {
                    flag = MoveFlag::MOVE_CASTLE_KING_SIDE;
                }
                else if (from == Square::E8 && to == Square::C8 && (m_ca_rights & CastleFlag::BQCA))
                {
                    flag = MoveFlag::MOVE_CASTLE_QUEEN_SIDE;
                }
                else
                {
                    flag = is_capture ? MoveFlag::MOVE_CAPTURE : MoveFlag::MOVE_QUIET;
                }
            }
        }
        else
        {
            flag = is_capture ? MoveFlag::MOVE_CAPTURE : MoveFlag::MOVE_QUIET;
        }

        const Move move(from, to, flag);

        return doMove(move);
    }

    void Position::doNullMove() {
        m_key ^=
          (m_ep_target == Square::NO_SQ) ? 0ULL : g_zobrist_table[k_ZOBRIST_EP_IDX][m_ep_target];
        m_checkers  = 0ULL;
        m_ep_target = Square::NO_SQ;
        m_stm       = colorFlip(m_stm);
        m_key ^= g_zobrist_side;
    }

    BitBoard Position::pieces(const Color c) const { return m_bb_colors[c]; }

    BitBoard Position::pieces(const PieceType pt) const { return m_bb_pieces[pt]; }

    BitBoard Position::pieces(const Color c, const PieceType pt) const {
        return m_bb_pieces[pt] & m_bb_colors[c];
    }

    BitBoard Position::occupied() const {
        return (m_bb_colors[Color::WHITE] | m_bb_colors[Color::BLACK]);
    }

    BitBoard Position::empty() const {
        return ~(m_bb_colors[Color::WHITE] | m_bb_colors[Color::BLACK]);
    }

    Piece Position::pieceOn(const Square square) const { return m_board[square]; }

    U8 Position::pieceCount(const PieceType pt) const { return m_bb_pieces[pt].count(); }

    U8 Position::pieceCount(const Piece piece) const {
        return (m_bb_pieces[pieceTypeOf(piece)] & m_bb_colors[pieceColorOf(piece)]).count();
    }

    Color Position::stm() const { return m_stm; }

    U8 Position::caRights() const { return m_ca_rights; }

    Square Position::epTarget() const { return m_ep_target; }

    U8 Position::halfmoves() const { return m_halfmoves; }

    U8 Position::fullmoves() const { return m_fullmoves; }

    U64 Position::key() const { return m_key; }

    U64 Position::pawnKey() const { return m_pawn_key; }

    bool Position::isValid() const {
        return (m_bb_pieces[PieceType::KING].count() == 2)
            && ((m_bb_pieces[PieceType::PAWN] & k_RANK_1_AND_8_BB).isEmpty());
    }

    bool Position::isInCheck() const { return !m_checkers.isEmpty(); }

    bool Position::isDrawn(std::span<U64> key_history) const {
        assert(key_history.size() == static_cast<size_t>(m_ply_count));

        const U64 key   = m_key;
        const I32 start = std::max(m_ply_count - m_halfmoves, 0);

        for (I32 i = m_ply_count - 2; i >= start; i -= 2)
        {
            if (key == key_history[i])
            {
                return true;
            }
        }
        return false;
    }

    void Position::display() const {
        std::ostringstream ss;

        for (I8 rank = Rank::RANK_8; rank >= Rank::RANK_1; rank--)
        {
            for (U8 file = File::FILE_A; file <= File::FILE_H; file++)
            {
                const Square sq = rf2sq(rank, file);
                ss << (char) k_PIECES_STR[this->m_board[sq]] << " ";
            }
            ss << "\n";
        }

        ss << "\n" << (char) k_COLORS_STR[m_stm];
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
