#include "position.h"
#include "movegen.h"
#include "utils.h"

namespace sagittar {

    namespace core {

        static uint64_t ZOBRIST_TABLE[16][64];  // [Piece][Square]
        static uint64_t ZOBRIST_CA[16];
        static uint64_t ZOBRIST_SIDE;
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

        template<Color US>
        constexpr int EP_DIR = (US == WHITE) ? 8 : -8;

        PositionHistory::PositionHistory() {
            hash_history.reserve(HISTORY_SIZE_MAX);
            hash_history.shrink_to_fit();
            hash_history.clear();
        }

        void PositionHistory::reset() { hash_history.clear(); }

        void PositionHistory::push(const uint64_t hash) { hash_history.emplace_back(hash); }

        uint64_t PositionHistory::peek(const size_t i) { return hash_history.at(i); }

        void PositionHistory::pop() { return hash_history.pop_back(); }

        Position::Position() :
            board(Board{}),
            black_to_play(false),
            ca_rights(0),
            ep_target(static_cast<Square>(0)),
            half_moves(0),
            full_moves(0),
            ply_count(0),
            hash(0ULL),
            king_sq(static_cast<Square>(0)),
            checkers(0ULL) {}

        void Position::reset() { *this = Position{}; }

        void Position::reset_hash() {
            hash = 0ULL;

            hash ^= ZOBRIST_SIDE & (-static_cast<uint64_t>(black_to_play));
            hash ^= ZOBRIST_CA[ca_rights];
            hash ^=
              ZOBRIST_TABLE[ZOBRIST_EP_IDX][ep_target] & (-static_cast<uint64_t>(!!ep_target));
            for (int sq = A1; sq <= H8; sq++)
            {
                const Piece p = board.pieces[sq];
                hash ^= ZOBRIST_TABLE[p][sq] & (-static_cast<uint64_t>(p != NO_PIECE));
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
                    board.set_piece(pt, color, RF_TO_SQ(rank, file++));
                }
            }

            assert(board.bb_pieces[PIECE_TYPE_INVALID] == 0ULL);

            // Active color
            ss >> segment;
            black_to_play = (segment == "b");

            // Castling rights
            ss >> segment;
            if (segment == "-")
            {
                ca_rights = CastlingRights::NOCA;
            }
            else
            {
                if (segment.find('K') != std::string::npos)
                {
                    ca_rights |= CastlingRights::WKCA;
                }
                if (segment.find('Q') != std::string::npos)
                {
                    ca_rights |= CastlingRights::WQCA;
                }
                if (segment.find('k') != std::string::npos)
                {
                    ca_rights |= CastlingRights::BKCA;
                }
                if (segment.find('q') != std::string::npos)
                {
                    ca_rights |= CastlingRights::BQCA;
                }
            }

            // En passant target
            ss >> segment;
            if (segment == "-")
            {
                ep_target = static_cast<Square>(0);
            }
            else
            {
                ep_target = static_cast<Square>(segment[0] - 'a' + 8 * (segment[1] - '1'));
            }

            // Halfmove clock
            ss >> segment;
            if (segment.empty() || segment == "-")
            {
                half_moves = 0;
            }
            else
            {
                half_moves = std::stoi(segment);
            }

            // Fullmove number
            ss >> segment;
            if (segment.empty() || segment == "-")
            {
                full_moves = 1;
            }
            else
            {
                full_moves = std::stoi(segment);
            }

#ifdef DEBUG
            board.assert_valid();
#endif

            reset_hash();

            const Color us   = static_cast<Color>(black_to_play);
            const Color them = COLOR_FLIP(us);

            // King square
            const BitBoard king_bb = board.bb_pieces[KING] & board.bb_colors[us];
            king_sq                = static_cast<Square>(__builtin_ctzll(king_bb));

            // Checkers
            checkers = movegen_get_square_attackers(this, king_sq, them);

            return board.is_valid();
        }

        bool Position::is_valid() const { return board.is_valid(); }

        bool Position::is_repeated(PositionHistory* const history) const {
            for (size_t i = std::max(ply_count - half_moves, static_cast<uint32_t>(0));
                 i < ply_count - 1; ++i)
            {
                if (hash == history->peek(i))
                    return true;
            }
            return false;
        }

        bool Position::is_in_check() const { return (checkers != 0ULL); }

        template<Color US, MoveFlag F>
        static bool _do_move(Position* const pos, const Move move) {
            constexpr bool  is_capture   = MOVE_IS_CAPTURE(F);
            constexpr bool  is_promotion = MOVE_IS_PROMOTION(F);
            constexpr Color them         = COLOR_FLIP(US);

            uint64_t hash_local = pos->hash;

            hash_local ^= SEL(pos->ep_target != 0ULL, static_cast<uint64_t>(0),
                              ZOBRIST_TABLE[ZOBRIST_EP_IDX][pos->ep_target]);

            pos->ep_target = static_cast<Square>(0);
            pos->half_moves++;
            pos->full_moves += pos->black_to_play;
            pos->ply_count++;

            const Square from = MOVE_FROM(move);
            const Square to   = MOVE_TO(move);

            auto board_ptr = &pos->board;

            const Piece     move_p  = board_ptr->pieces[from];
            const PieceType move_pt = PIECE_TYPE_OF(move_p);

            const Piece captured_p = board_ptr->pieces[to];

            const BitBoard move_mask_to = BB(to);
            const BitBoard move_mask    = BB(from) | move_mask_to;

            board_ptr->bb_pieces[move_pt] ^= move_mask;
            board_ptr->bb_colors[US] ^= move_mask;
            board_ptr->pieces[from] = NO_PIECE;
            board_ptr->pieces[to]   = move_p;
            hash_local ^= ZOBRIST_TABLE[move_p][from];
            hash_local ^= ZOBRIST_TABLE[move_p][to];

            if constexpr ((F == MOVE_CASTLE_KING_SIDE) || (F == MOVE_CASTLE_QUEEN_SIDE))
            {
                assert(move_pt == KING);
                constexpr Piece    rook           = PIECE_CREATE(ROOK, US);
                constexpr Rank     rank           = (US == WHITE) ? RANK_1 : RANK_8;
                constexpr File     from_file      = (F == MOVE_CASTLE_KING_SIDE) ? FILE_H : FILE_A;
                constexpr File     to_file        = (F == MOVE_CASTLE_KING_SIDE) ? FILE_F : FILE_D;
                constexpr Square   ca_r_from_sq   = RF_TO_SQ(rank, from_file);
                constexpr Square   ca_r_to_sq     = RF_TO_SQ(rank, to_file);
                constexpr BitBoard move_mask_ca_r = (BB(ca_r_from_sq) | BB(ca_r_to_sq));
                board_ptr->bb_pieces[ROOK] ^= move_mask_ca_r;
                board_ptr->bb_colors[US] ^= move_mask_ca_r;
                board_ptr->pieces[ca_r_from_sq] = NO_PIECE;
                board_ptr->pieces[ca_r_to_sq]   = rook;
                hash_local ^= ZOBRIST_TABLE[rook][ca_r_from_sq];
                hash_local ^= ZOBRIST_TABLE[rook][ca_r_to_sq];
            }
            else if constexpr (F == MOVE_QUIET_PAWN_DBL_PUSH)
            {
                assert(move_pt == PAWN);
                pos->ep_target = static_cast<Square>(from + EP_DIR<US>);
                hash_local ^= ZOBRIST_TABLE[ZOBRIST_EP_IDX][pos->ep_target];
            }
            else
            {
                if constexpr (is_capture)
                {
                    if constexpr (F == MOVE_CAPTURE_EP)
                    {
                        assert(move_pt == PAWN);
                        assert(captured_p == NO_PIECE);
                        constexpr Piece ep_victim       = PIECE_CREATE(PAWN, them);
                        const Square    ep_victim_sq    = static_cast<Square>(to - EP_DIR<US>);
                        const BitBoard  ep_victim_sq_bb = BB(ep_victim_sq);
                        board_ptr->bb_pieces[PAWN] ^= ep_victim_sq_bb;
                        board_ptr->bb_colors[them] ^= ep_victim_sq_bb;
                        board_ptr->pieces[ep_victim_sq] = NO_PIECE;
                        hash_local ^= ZOBRIST_TABLE[ep_victim][ep_victim_sq];
                    }
                    else
                    {
                        assert(captured_p != NO_PIECE);
                        const PieceType captured_pt = PIECE_TYPE_OF(captured_p);
                        board_ptr->bb_pieces[captured_pt] ^= move_mask_to;
                        board_ptr->bb_colors[them] ^= move_mask_to;
                        hash_local ^= ZOBRIST_TABLE[captured_p][to];
                    }
                }

                if constexpr (is_promotion)
                {
                    assert(move_pt == PAWN);
                    const PieceType promoted_pt = MOVE_PROMOTED_PIECE_TYPE(move);
                    assert((promoted_pt != PAWN) && (promoted_pt != KING));
                    const Piece promoted = PIECE_CREATE(promoted_pt, US);
                    board_ptr->bb_pieces[move_pt] ^= move_mask_to;
                    board_ptr->bb_pieces[promoted_pt] ^= move_mask_to;
                    board_ptr->pieces[to] = PIECE_CREATE(promoted_pt, US);
                    hash_local ^= ZOBRIST_TABLE[move_p][to];
                    hash_local ^= ZOBRIST_TABLE[promoted][to];
                }
            }

            assert(board_ptr->bb_pieces[PIECE_TYPE_INVALID] == 0ULL);

            pos->half_moves *= !((move_pt == PAWN) || is_capture);

            hash_local ^= ZOBRIST_CA[pos->ca_rights];
            pos->ca_rights &= CASTLE_RIGHTS_MODIFIERS[from];
            hash_local ^= ZOBRIST_CA[pos->ca_rights];

            const BitBoard k_bb = board_ptr->bb_pieces[KING];

            const BitBoard king_bb_us  = k_bb & board_ptr->bb_colors[US];
            const Square   king_sq_us  = static_cast<Square>(__builtin_ctzll(king_bb_us));
            const BitBoard checkers_us = movegen_get_square_attackers(pos, king_sq_us, them);

            const bool is_valid_move = (checkers_us == 0ULL) && board_ptr->is_valid();

            const BitBoard king_bb_them = k_bb & board_ptr->bb_colors[them];
            pos->king_sq                = static_cast<Square>(__builtin_ctzll(king_bb_them));
            pos->checkers               = movegen_get_square_attackers(pos, pos->king_sq, US);

            pos->black_to_play = !pos->black_to_play;
            hash_local ^= ZOBRIST_SIDE;

            pos->hash = hash_local;

#ifdef DEBUG
            board_ptr->assert_valid();
            const uint64_t currhash = pos->hash;
            pos->reset_hash();
            assert(currhash == pos->hash);
#endif

            return is_valid_move;
        }

        template<Color US>
        using DoMoveFn = bool (*)(Position*, Move);

        template<Color US>
        constexpr std::array<DoMoveFn<US>, 16> do_move_dispatch_table = [] {
            std::array<DoMoveFn<US>, 16> table{};

            table[MOVE_QUIET]                    = _do_move<US, MOVE_QUIET>;
            table[MOVE_QUIET_PAWN_DBL_PUSH]      = _do_move<US, MOVE_QUIET_PAWN_DBL_PUSH>;
            table[MOVE_CASTLE_KING_SIDE]         = _do_move<US, MOVE_CASTLE_KING_SIDE>;
            table[MOVE_CASTLE_QUEEN_SIDE]        = _do_move<US, MOVE_CASTLE_QUEEN_SIDE>;
            table[MOVE_CAPTURE]                  = _do_move<US, MOVE_CAPTURE>;
            table[MOVE_CAPTURE_EP]               = _do_move<US, MOVE_CAPTURE_EP>;
            table[MOVE_PROMOTION_KNIGHT]         = _do_move<US, MOVE_PROMOTION_KNIGHT>;
            table[MOVE_PROMOTION_BISHOP]         = _do_move<US, MOVE_PROMOTION_BISHOP>;
            table[MOVE_PROMOTION_ROOK]           = _do_move<US, MOVE_PROMOTION_ROOK>;
            table[MOVE_PROMOTION_QUEEN]          = _do_move<US, MOVE_PROMOTION_QUEEN>;
            table[MOVE_CAPTURE_PROMOTION_KNIGHT] = _do_move<US, MOVE_CAPTURE_PROMOTION_KNIGHT>;
            table[MOVE_CAPTURE_PROMOTION_BISHOP] = _do_move<US, MOVE_CAPTURE_PROMOTION_BISHOP>;
            table[MOVE_CAPTURE_PROMOTION_ROOK]   = _do_move<US, MOVE_CAPTURE_PROMOTION_ROOK>;
            table[MOVE_CAPTURE_PROMOTION_QUEEN]  = _do_move<US, MOVE_CAPTURE_PROMOTION_QUEEN>;

            return table;
        }();

        static bool _do_move_wrapper(Position* pos, const Move move) {
            const MoveFlag flag = MOVE_FLAG(move);
            return pos->black_to_play ? do_move_dispatch_table<BLACK>[flag](pos, move)
                                      : do_move_dispatch_table<WHITE>[flag](pos, move);
        }

        bool Position::do_move(const Move move, PositionHistory* const history) {
            history->push(hash);
            const Color  us               = static_cast<Color>(black_to_play);
            const Square from             = MOVE_FROM(move);
            const Color  move_piece_color = PIECE_COLOR_OF(board.pieces[from]);
            const bool   valid            = (move_piece_color == us);
            return valid && _do_move_wrapper(this, move);
        }

        bool Position::do_move(const std::string& move_str, PositionHistory* const history) {
            const std::size_t len = move_str.length();
            if (len < 4 || len > 5) [[unlikely]]
                return false;

            const int from_file = move_str[0] - 'a';
            const int from_rank = (move_str[1] - '0') - 1;
            const int to_file   = move_str[2] - 'a';
            const int to_rank   = (move_str[3] - '0') - 1;

            if (from_file < FILE_A || from_file > FILE_H) [[unlikely]]
                return false;

            if (to_file < FILE_A || to_file > FILE_H) [[unlikely]]
                return false;

            if (from_rank < RANK_1 || from_rank > RANK_8) [[unlikely]]
                return false;

            if (to_rank < RANK_1 || to_rank > RANK_8) [[unlikely]]
                return false;

            const Square from = RF_TO_SQ(from_rank, from_file);
            const Square to   = RF_TO_SQ(to_rank, to_file);

            const Piece move_p = board.pieces[from];
            if (move_p == NO_PIECE) [[unlikely]]
                return false;

            const Color us = static_cast<Color>(black_to_play);

            if (PIECE_COLOR_OF(move_p) != us) [[unlikely]]
                return false;

            const PieceType move_pt = PIECE_TYPE_OF(move_p);

            const bool is_promotion = (len == 5);

            if (is_promotion)
            {
                if (move_pt != PAWN) [[unlikely]]
                    return false;

                if (black_to_play)
                {
                    if (from_rank != RANK_2 || to_rank != RANK_1) [[unlikely]]
                        return false;
                }
                else
                {
                    if (from_rank != RANK_7 || to_rank != RANK_8) [[unlikely]]
                        return false;
                }

                const bool is_valid_promo_pt = (move_str[4] == 'q') || (move_str[4] == 'r')
                                            || (move_str[4] == 'n') || (move_str[4] == 'b');
                if (!is_valid_promo_pt) [[unlikely]]
                    return false;
            }

            const Piece captured_p = board.pieces[to];
            const bool  is_capture = (captured_p != NO_PIECE);

            if (is_capture)
            {
                if ((PIECE_COLOR_OF(captured_p) == us) || (PIECE_TYPE_OF(captured_p) == KING))
                  [[unlikely]]
                    return false;
            }

            MoveFlag flag;

            if (move_pt == PAWN)
            {
                if (!is_promotion)
                {
                    if (!is_capture)
                    {
                        if (us == WHITE && from_rank == RANK_2 && to_rank == RANK_4)
                            flag = MOVE_QUIET_PAWN_DBL_PUSH;
                        else if (us == BLACK && from_rank == RANK_7 && to_rank == RANK_5)
                            flag = MOVE_QUIET_PAWN_DBL_PUSH;
                        else if (to == ep_target)
                            flag = MOVE_CAPTURE_EP;
                        else
                            flag = MOVE_QUIET;
                    }
                    else
                        flag = MOVE_CAPTURE;
                }
                else
                {
                    const char promoted_ch = move_str[4];
                    int        promo_flag  = (promoted_ch == 'q') ? MOVE_PROMOTION_QUEEN
                                           : (promoted_ch == 'r') ? MOVE_PROMOTION_ROOK
                                           : (promoted_ch == 'n') ? MOVE_PROMOTION_KNIGHT
                                           : (promoted_ch == 'b') ? MOVE_PROMOTION_BISHOP
                                                                  : MOVE_PROMOTION_QUEEN;
                    if (is_capture)
                        promo_flag |= MOVE_CAPTURE;
                    flag = static_cast<MoveFlag>(promo_flag);
                }
            }
            else if (move_pt == KING)
            {
                if (us == WHITE)
                {
                    if (from == E1 && to == G1)
                    {
                        if ((ca_rights & WKCA) == 0) [[unlikely]]
                            return false;
                        flag = MOVE_CASTLE_KING_SIDE;
                    }
                    else if (from == E1 && to == C1)
                    {
                        if ((ca_rights & WQCA) == 0) [[unlikely]]
                            return false;
                        flag = MOVE_CASTLE_QUEEN_SIDE;
                    }
                    else
                        flag = is_capture ? MOVE_CAPTURE : MOVE_QUIET;
                }
                else
                {
                    if (from == E8 && to == G8)
                    {
                        if ((ca_rights & BKCA) == 0) [[unlikely]]
                            return false;
                        flag = MOVE_CASTLE_KING_SIDE;
                    }
                    else if (from == E8 && to == C8)
                    {
                        if ((ca_rights & BQCA) == 0) [[unlikely]]
                            return false;
                        flag = MOVE_CASTLE_QUEEN_SIDE;
                    }
                    else
                        flag = is_capture ? MOVE_CAPTURE : MOVE_QUIET;
                }
            }
            else
            {
                flag = is_capture ? MOVE_CAPTURE : MOVE_QUIET;
            }

            const Move move = MOVE_CREATE(from, to, flag);

            return do_move(move, history);
        }

        void Position::undo_move(PositionHistory* const history) { history->pop(); }

        void Position::display() const {
            board.display();

            const char active_color = black_to_play ? 'b' : 'w';
            std::cout << "\nActive Color\t\t: " << (char) active_color << "\n";
            std::cout << "Castling Rights\t\t: ";
            if (ca_rights == NOCA)
            {
                std::cout << "-";
            }
            else
            {
                if (ca_rights & WKCA)
                {
                    std::cout << "K";
                }
                if (ca_rights & WQCA)
                {
                    std::cout << "Q";
                }
                if (ca_rights & BKCA)
                {
                    std::cout << "k";
                }
                if (ca_rights & BQCA)
                {
                    std::cout << "q";
                }
            }
            std::cout << "\n";
            std::cout << "En passant Target\t: ";
            if (ep_target)
            {
                std::cout << (char) SQ_TO_FILE(ep_target) + 'a' << (int) SQ_TO_RANK(ep_target) + 1;
            }
            else
            {
                std::cout << "-";
            }
            std::cout << "\n";
            std::cout << "Halfmove Clock\t\t: " << (int) half_moves << "\n";
            std::cout << "Fullmove Number\t\t: " << (int) full_moves << "\n";
            std::cout << "Hash\t\t\t: " << (uint64_t) hash << "\n";
            std::cout << "Checkers\t\t: " << (uint64_t) checkers << "\n" << std::endl;
        }

        void position_init() {
            // Zobrist
            for (int p = 0; p < 16; p++)
            {
                for (int sq = A1; sq <= H8; sq++)
                {
                    ZOBRIST_TABLE[p][sq] = prng();
                }
            }
            for (int i = 0; i < 16; i++)
            {
                ZOBRIST_CA[i] = prng();
            }
            ZOBRIST_SIDE = prng();
        }
    }
}
