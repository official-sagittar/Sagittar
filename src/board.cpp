#include "board.h"
#include "fen.h"
#include "movegen.h"
#include "utils.h"

namespace sagittar {

    namespace board {

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

        void Board::initialize() {

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

        Board::Board() { reset(); }

        Board::~Board() {}

        void Board::reset() {
            for (auto& bb : bitboards)
            {
                bb = 0ULL;
            }

            for (auto& p : pieces)
            {
                p = Piece::NO_PIECE;
            }

            bitboards[Piece::NO_PIECE] = 0xFFFFFFFFFFFFFFFF;

            checkers = 0ULL;

            active_color     = Color::WHITE;
            casteling_rights = CastleFlag::NOCA;
            enpassant_target = Square::NO_SQ;
            half_move_clock  = 0;
            full_move_number = 0;
            ply_count        = 0;
            hash             = 0ULL;
            history          = {};
        }

        void Board::resetHash() {
            hash = 0ULL;

            if (active_color == Color::WHITE)
            {
                hash ^= ZOBRIST_SIDE;
            }

            hash ^= ZOBRIST_CA[casteling_rights];

            for (u8 sq = Square::A1; sq <= Square::H8; sq++)
            {
                const Piece p = pieces[sq];

                if (p == Piece::NO_PIECE)
                {
                    continue;
                }

                hash ^= ZOBRIST_TABLE[p][sq];
            }
        }

        void Board::setPiece(const Piece piece, const Square square) {
#ifdef DEBUG
            assert(piece != Piece::NO_PIECE);
            assert(square != Square::NO_SQ);
#endif
            const BitBoard bit          = 1ULL << square;
            const BitBoard bit_inverted = ~(bit);
            bitboards[piece] |= bit;
            bitboards[Piece::NO_PIECE] &= bit_inverted;
            bitboards[bitboardColorSlot(piece)] |= bit;
            pieces[square] = piece;
            hash ^= ZOBRIST_TABLE[piece][square];
        }

        void Board::clearPiece(const Piece piece, const Square square) {
#ifdef DEBUG
            assert(piece != Piece::NO_PIECE);
            assert(square != Square::NO_SQ);
#endif
            const BitBoard bit          = 1ULL << square;
            const BitBoard bit_inverted = ~(bit);
            bitboards[piece] &= bit_inverted;
            bitboards[Piece::NO_PIECE] |= bit;
            bitboards[bitboardColorSlot(piece)] &= bit_inverted;
            pieces[square] = Piece::NO_PIECE;
            hash ^= ZOBRIST_TABLE[piece][square];
        }

        void Board::setStartpos() {
            reset();

            setPiece(Piece::WHITE_ROOK, Square::A1);
            setPiece(Piece::WHITE_KNIGHT, Square::B1);
            setPiece(Piece::WHITE_BISHOP, Square::C1);
            setPiece(Piece::WHITE_QUEEN, Square::D1);
            setPiece(Piece::WHITE_KING, Square::E1);
            setPiece(Piece::WHITE_BISHOP, Square::F1);
            setPiece(Piece::WHITE_KNIGHT, Square::G1);
            setPiece(Piece::WHITE_ROOK, Square::H1);

            setPiece(Piece::WHITE_PAWN, Square::A2);
            setPiece(Piece::WHITE_PAWN, Square::B2);
            setPiece(Piece::WHITE_PAWN, Square::C2);
            setPiece(Piece::WHITE_PAWN, Square::D2);
            setPiece(Piece::WHITE_PAWN, Square::E2);
            setPiece(Piece::WHITE_PAWN, Square::F2);
            setPiece(Piece::WHITE_PAWN, Square::G2);
            setPiece(Piece::WHITE_PAWN, Square::H2);

            setPiece(Piece::BLACK_ROOK, Square::A8);
            setPiece(Piece::BLACK_KNIGHT, Square::B8);
            setPiece(Piece::BLACK_BISHOP, Square::C8);
            setPiece(Piece::BLACK_QUEEN, Square::D8);
            setPiece(Piece::BLACK_KING, Square::E8);
            setPiece(Piece::BLACK_BISHOP, Square::F8);
            setPiece(Piece::BLACK_KNIGHT, Square::G8);
            setPiece(Piece::BLACK_ROOK, Square::H8);

            setPiece(Piece::BLACK_PAWN, Square::A7);
            setPiece(Piece::BLACK_PAWN, Square::B7);
            setPiece(Piece::BLACK_PAWN, Square::C7);
            setPiece(Piece::BLACK_PAWN, Square::D7);
            setPiece(Piece::BLACK_PAWN, Square::E7);
            setPiece(Piece::BLACK_PAWN, Square::F7);
            setPiece(Piece::BLACK_PAWN, Square::G7);
            setPiece(Piece::BLACK_PAWN, Square::H7);

            active_color = Color::WHITE;
            casteling_rights =
              CastleFlag::WKCA | CastleFlag::WQCA | CastleFlag::BKCA | CastleFlag::BQCA;
            enpassant_target = Square::NO_SQ;
            half_move_clock  = 0;
            full_move_number = 1;
            ply_count        = 0;

            resetHash();
        }

        void Board::movePiece(const Piece  piece,
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
                const Piece captured = pieces[to];
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
                assert(pieces[to] == Piece::NO_PIECE);
#endif
                setPiece(promoted, to);
            }
            else
            {
#ifdef DEBUG
                assert(pieces[to] == Piece::NO_PIECE);
#endif
                setPiece(piece, to);
            }
        }

        void Board::undoMovePiece(const Piece  piece,
                                  const Square from,
                                  const Square to,
                                  const bool   is_capture,
                                  const Piece  captured,
                                  const bool   is_promotion,
                                  const Piece  promoted) {

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
#endif
                clearPiece(promoted, to);
            }
            else
            {
                clearPiece(piece, to);
            }

            if (is_capture)
            {
#ifdef DEBUG
                assert(captured != Piece::NO_PIECE);
                assert(pieceColorOf(piece) == colorFlip(pieceColorOf(captured)));
#endif
                setPiece(captured, to);
            }

#ifdef DEBUG
            assert(pieces[from] == Piece::NO_PIECE);
#endif
            setPiece(piece, from);
        }

        void Board::setCheckers(const BitBoard bb) { checkers = bb; }

        void Board::setActiveColor(const Color c) { active_color = c; }

        void Board::addCastelingRights(const CastleFlag f) { casteling_rights |= f; }

        void Board::setEnpassantTarget(const Square s) { enpassant_target = s; }

        void Board::setHalfmoveClock(const u8 n) { half_move_clock = n; }

        void Board::setFullmoveNumber(const u8 n) { full_move_number = n; }

        DoMoveResult Board::doMoveComplete() {
            const Color them = colorFlip(active_color);

            // Check if move does not leave our King in check and board is valid
            Piece           king          = pieceCreate(PieceType::KING, active_color);
            board::BitBoard bb            = bitboards[king];
            Square          sq            = static_cast<Square>(utils::bitScanForward(&bb));
            const BitBoard  checkers_us   = movegen::getSquareAttackers(*this, sq, them);
            const bool      is_valid_move = (checkers_us == 0ULL) && isValid();

            // Set checkers
            king     = pieceCreate(PieceType::KING, them);
            bb       = bitboards[king];
            sq       = static_cast<Square>(utils::bitScanForward(&bb));
            checkers = movegen::getSquareAttackers(*this, sq, active_color);

            // Switch sides
            active_color = them;
            hash ^= ZOBRIST_SIDE;
#ifdef DEBUG
            const u64 currhash = hash;
            resetHash();
            assert(currhash == hash);
#endif
            return is_valid_move ? DoMoveResult::LEGAL : DoMoveResult::ILLEGAL;
        }

        [[nodiscard]] DoMoveResult Board::doMove(const move::Move move) noexcept {
            const Square         from     = move.getFrom();
            const Square         to       = move.getTo();
            const move::MoveFlag flag     = move.getFlag();
            const Piece          piece    = pieces[from];
            const Piece          captured = pieces[to];

            if (pieceColorOf(piece) == colorFlip(active_color)) [[unlikely]]
            {
                return DoMoveResult::INVALID;
            }

            history.emplace_back(move, captured, casteling_rights, enpassant_target,
                                 half_move_clock, full_move_number, hash, checkers);

            enpassant_target = Square::NO_SQ;
            half_move_clock++;
            if (active_color == Color::BLACK)
            {
                full_move_number++;
            }
            ply_count++;

            if (flag == move::MoveFlag::MOVE_QUIET_PAWN_DBL_PUSH)
            {
                movePiece(piece, from, to);
                const i8 stm     = 1 - (2 * active_color);  // WHITE = 1; BLACK = -1
                enpassant_target = static_cast<Square>(from + (8 * stm));
                half_move_clock  = 0;
                return doMoveComplete();
            }
            else if (flag == move::MoveFlag::MOVE_CAPTURE_EP)
            {
                movePiece(piece, from, to);
                const i8     stm         = -1 + (2 * active_color);  // WHITE = -1; BLACK = 1
                const Square captured_sq = static_cast<Square>(to + (8 * stm));
                const Piece  captured    = pieceCreate(PieceType::PAWN, colorFlip(active_color));
                clearPiece(captured, captured_sq);
                half_move_clock = 0;
                return doMoveComplete();
            }
            else if (flag == move::MoveFlag::MOVE_CASTLE_KING_SIDE)
            {
                movePiece(piece, from, to);
                const Piece rook = pieceCreate(PieceType::ROOK, active_color);
                movePiece(rook, static_cast<Square>(to + 1), static_cast<Square>(to - 1));
                hash ^= ZOBRIST_CA[casteling_rights];
                casteling_rights &= CASTLE_RIGHTS_MODIFIERS[from];
                hash ^= ZOBRIST_CA[casteling_rights];
                return doMoveComplete();
            }
            else if (flag == move::MoveFlag::MOVE_CASTLE_QUEEN_SIDE)
            {
                movePiece(piece, from, to);
                const Piece rook = pieceCreate(PieceType::ROOK, active_color);
                movePiece(rook, static_cast<Square>(to - 2), static_cast<Square>(to + 1));
                hash ^= ZOBRIST_CA[casteling_rights];
                casteling_rights &= CASTLE_RIGHTS_MODIFIERS[from];
                hash ^= ZOBRIST_CA[casteling_rights];
                return doMoveComplete();
            }

            Piece promoted;

            switch (flag)
            {
                case move::MoveFlag::MOVE_PROMOTION_KNIGHT :
                case move::MoveFlag::MOVE_CAPTURE_PROMOTION_KNIGHT :
                    promoted = pieceCreate(PieceType::KNIGHT, active_color);
                    break;
                case move::MoveFlag::MOVE_PROMOTION_BISHOP :
                case move::MoveFlag::MOVE_CAPTURE_PROMOTION_BISHOP :
                    promoted = pieceCreate(PieceType::BISHOP, active_color);
                    break;
                case move::MoveFlag::MOVE_PROMOTION_ROOK :
                case move::MoveFlag::MOVE_CAPTURE_PROMOTION_ROOK :
                    promoted = pieceCreate(PieceType::ROOK, active_color);
                    break;
                case move::MoveFlag::MOVE_PROMOTION_QUEEN :
                case move::MoveFlag::MOVE_CAPTURE_PROMOTION_QUEEN :
                    promoted = pieceCreate(PieceType::QUEEN, active_color);
                    break;
                default :
                    promoted = Piece::NO_PIECE;
            }

            movePiece(piece, from, to, move::isCapture(flag), move::isPromotion(flag), promoted);

            if ((pieceTypeOf(piece) == PieceType::PAWN) || move::isCapture(flag))
            {
                half_move_clock = 0;
            }

            hash ^= ZOBRIST_CA[casteling_rights];
            casteling_rights &= (CASTLE_RIGHTS_MODIFIERS[from] & CASTLE_RIGHTS_MODIFIERS[to]);
            hash ^= ZOBRIST_CA[casteling_rights];

            return doMoveComplete();
        }

        [[nodiscard]] DoMoveResult Board::doMove(const std::string& move_str) noexcept {
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
                && (from_rank != promotionRankSrcOf(active_color)
                    || to_rank != promotionRankDestOf(active_color)))
            {
                return DoMoveResult::INVALID;
            }

            const Square from       = rf2sq(from_rank, from_file);
            const Square to         = rf2sq(to_rank, to_file);
            const Piece  piece      = pieces[from];
            const bool   is_capture = (pieces[to] != Piece::NO_PIECE);

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
                        if (active_color == Color::WHITE && from_rank == Rank::RANK_2
                            && to_rank == Rank::RANK_4)
                        {
                            flag = move::MoveFlag::MOVE_QUIET_PAWN_DBL_PUSH;
                        }
                        else if (active_color == Color::BLACK && from_rank == Rank::RANK_7
                                 && to_rank == Rank::RANK_5)
                        {
                            flag = move::MoveFlag::MOVE_QUIET_PAWN_DBL_PUSH;
                        }
                        else if (to == enpassant_target)
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
            else if (pieceTypeOf(pieces[from]) == PieceType::KING)
            {
                if (active_color == Color::WHITE)
                {
                    if (from == Square::E1 && to == Square::G1
                        && (casteling_rights & CastleFlag::WKCA))
                    {
                        flag = move::MoveFlag::MOVE_CASTLE_KING_SIDE;
                    }
                    else if (from == Square::E1 && to == Square::C1
                             && (casteling_rights & CastleFlag::WQCA))
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
                    if (from == Square::E8 && to == Square::G8
                        && (casteling_rights & CastleFlag::BKCA))
                    {
                        flag = move::MoveFlag::MOVE_CASTLE_KING_SIDE;
                    }
                    else if (from == Square::E8 && to == Square::C8
                             && (casteling_rights & CastleFlag::BQCA))
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

        void Board::doNullMove() {
            const move::Move nullmove;

            history.emplace_back(nullmove, Piece::NO_PIECE, casteling_rights, enpassant_target,
                                 half_move_clock, full_move_number, hash, checkers);

            checkers         = 0ULL;
            enpassant_target = Square::NO_SQ;
            active_color     = colorFlip(active_color);
            hash ^= ZOBRIST_SIDE;
        }

        void Board::undoMove() {
            const MoveHistoryEntry& history_entry = history.top();

            const move::Move     move              = history_entry.move;
            const Square         from              = move.getFrom();
            const Square         to                = move.getTo();
            const move::MoveFlag flag              = move.getFlag();
            Piece                piece             = pieces[to];
            const Piece          captured          = history_entry.captured;
            const Color          prev_active_color = colorFlip(active_color);

            if (flag == move::MoveFlag::MOVE_CASTLE_KING_SIDE)
            {
                undoMovePiece(piece, from, to);
                const Piece rook = pieceCreate(PieceType::ROOK, prev_active_color);
                undoMovePiece(rook, static_cast<Square>(to + 1), static_cast<Square>(to - 1));
            }
            else if (flag == move::MoveFlag::MOVE_CASTLE_QUEEN_SIDE)
            {
                undoMovePiece(piece, from, to);
                const Piece rook = pieceCreate(PieceType::ROOK, prev_active_color);
                undoMovePiece(rook, static_cast<Square>(to - 2), static_cast<Square>(to + 1));
            }
            else if (flag == move::MoveFlag::MOVE_CAPTURE_EP)
            {
                undoMovePiece(piece, from, to);
                const i8     stm         = 1 - (2 * active_color);  // WHITE = 1; BLACK = -1
                const Square captured_sq = static_cast<Square>(to + (8 * stm));
                const Piece  captured    = pieceCreate(PieceType::PAWN, active_color);
                setPiece(captured, captured_sq);
            }
            else
            {
                const Piece promoted = move::isPromotion(flag) ? pieces[to] : Piece::NO_PIECE;
                piece =
                  move::isPromotion(flag) ? pieceCreate(PieceType::PAWN, prev_active_color) : piece;
                undoMovePiece(piece, from, to, move::isCapture(flag), captured,
                              move::isPromotion(flag), promoted);
            }

            hash ^= ZOBRIST_CA[casteling_rights];
            hash ^= ZOBRIST_CA[history_entry.casteling_rights];

            hash ^= ZOBRIST_SIDE;

#ifdef DEBUG
            assert(hash == history_entry.hash);
            assert(colorFlip(active_color) == prev_active_color);
#endif

            checkers         = history_entry.checkers;
            active_color     = prev_active_color;
            casteling_rights = history_entry.casteling_rights;
            enpassant_target = history_entry.enpassant_target;
            half_move_clock  = history_entry.half_move_clock;
            full_move_number = history_entry.full_move_number;
            ply_count--;

            history.pop();
        }

        void Board::undoNullMove() {
            const MoveHistoryEntry& history_entry = history.top();

            const Color prev_active_color = colorFlip(active_color);

            hash ^= ZOBRIST_SIDE;

#ifdef DEBUG
            assert(hash == history_entry.hash);
            assert(colorFlip(active_color) == prev_active_color);
#endif

            checkers         = history_entry.checkers;
            active_color     = prev_active_color;
            casteling_rights = history_entry.casteling_rights;
            enpassant_target = history_entry.enpassant_target;
            half_move_clock  = history_entry.half_move_clock;
            full_move_number = history_entry.full_move_number;

            history.pop();
        }

        BitBoard Board::getBitboard(const u8 index) const { return bitboards[index]; }

        BitBoard Board::getBitboard(const PieceType pt, const Color c) const {
            return bitboards[pieceCreate(pt, c)];
        }

        Piece Board::getPiece(const Square square) const { return pieces[square]; }

        u8 Board::getPieceCount(const Piece piece) const {
            return utils::bitCount1s(bitboards[piece]);
        }

        Color Board::getActiveColor() const { return active_color; }

        u8 Board::getCastelingRights() const { return casteling_rights; }

        Square Board::getEnpassantTarget() const { return enpassant_target; }

        u8 Board::getHalfmoveClock() const { return half_move_clock; }

        u8 Board::getFullmoveNumber() const { return full_move_number; }

        u64 Board::getHash() const { return hash; }

        bool Board::isValid() const {
            const bool check = (getPieceCount(Piece::WHITE_KING) == 1)
                            && (getPieceCount(Piece::BLACK_KING) == 1)
                            && (!(bitboards[Piece::WHITE_PAWN] & MASK_RANK_1))
                            && (!(bitboards[Piece::WHITE_PAWN] & MASK_RANK_8))
                            && (!(bitboards[Piece::BLACK_PAWN] & MASK_RANK_1))
                            && (!(bitboards[Piece::BLACK_PAWN] & MASK_RANK_8));
            return check;
        }

        bool Board::isInCheck() const { return (checkers != 0ULL); }

        bool Board::hasPositionRepeated(std::span<u64> key_history) const {
            for (u8 i = std::max(ply_count - half_move_clock, 0); i < ply_count - 1; ++i)
            {
                if (hash == key_history[i])
                {
                    return true;
                }
            }
            return false;
        }

        bool Board::operator==(Board const& rhs) const { return hash == rhs.getHash(); }

        void Board::display() const {
            std::ostringstream ss;

            for (i8 rank = Rank::RANK_8; rank >= Rank::RANK_1; rank--)
            {
                for (u8 file = File::FILE_A; file <= File::FILE_H; file++)
                {
                    const Square sq = rf2sq(rank, file);
                    ss << (char) PIECES_STR[this->pieces[sq]] << " ";
                }
                ss << "\n";
            }

            ss << "\n" << (char) COLORS_STR[active_color];
            ss << " " << (int) casteling_rights;
            ss << " " << (int) enpassant_target;
            ss << " " << (int) half_move_clock;
            ss << " " << (int) full_move_number;
            ss << " " << (int) ply_count;
            ss << " " << (unsigned long long) hash << "\n";

            std::cout << ss.str() << fen::toFEN(*this) << std::endl;
        }

    }

}
