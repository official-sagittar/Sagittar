#include "movegen.h"
#include "utils.h"

namespace sagittar {

    namespace movegen {

        static constexpr unsigned int MAGIC_MAX_TRIES = 500000;

        struct Magic {
            board::BitBoard mask;
            u64             magic;
            u8              shift;
        };

        static Magic MAGICTABLE_BISHOP[64];
        static Magic MAGICTABLE_ROOK[64];

        static board::BitBoard ATTACK_TABLE_PAWN[2][64];
        static board::BitBoard ATTACK_TABLE_KNIGHT[64];
        static board::BitBoard ATTACK_TABLE_BISHOP[64][512];
        static board::BitBoard ATTACK_TABLE_ROOK[64][4096];
        static board::BitBoard ATTACK_TABLE_KING[64];

        static board::BitBoard bishopMask(const Square sq) {
            i8 r, f;

            const u8 tr = sq2rank(sq);
            const u8 tf = sq2file(sq);

            board::BitBoard attack_mask = 0ULL;

            for (r = tr + 1, f = tf + 1; r <= 6 && f <= 6; r++, f++)
            {
                attack_mask |= (1ULL << rf2sq(r, f));
            }
            for (r = tr + 1, f = tf - 1; r <= 6 && f >= 1; r++, f--)
            {
                attack_mask |= (1ULL << rf2sq(r, f));
            }
            for (r = tr - 1, f = tf + 1; r >= 1 && f <= 6; r--, f++)
            {
                attack_mask |= (1ULL << rf2sq(r, f));
            }
            for (r = tr - 1, f = tf - 1; r >= 1 && f >= 1; r--, f--)
            {
                attack_mask |= (1ULL << rf2sq(r, f));
            }

            return attack_mask;
        }

        static board::BitBoard bishopAttacks(const Square sq, const board::BitBoard blockers) {
            i8 r, f;

            const u8 tr = sq2rank(sq);
            const u8 tf = sq2file(sq);

            board::BitBoard attack_mask = 0ULL;
            board::BitBoard sqb;

            for (r = tr + 1, f = tf + 1; r <= 7 && f <= 7; r++, f++)
            {
                sqb = 1ULL << rf2sq(r, f);
                attack_mask |= sqb;
                if (blockers & sqb)
                {
                    break;
                }
            }
            for (r = tr + 1, f = tf - 1; r <= 7 && f >= 0; r++, f--)
            {
                sqb = 1ULL << rf2sq(r, f);
                attack_mask |= sqb;
                if (blockers & sqb)
                {
                    break;
                }
            }
            for (r = tr - 1, f = tf + 1; r >= 0 && f <= 7; r--, f++)
            {
                sqb = 1ULL << rf2sq(r, f);
                attack_mask |= sqb;
                if (blockers & sqb)
                {
                    break;
                }
            }
            for (r = tr - 1, f = tf - 1; r >= 0 && f >= 0; r--, f--)
            {
                sqb = 1ULL << rf2sq(r, f);
                attack_mask |= sqb;
                if (blockers & sqb)
                {
                    break;
                }
            }

            return attack_mask;
        }

        static board::BitBoard rookMask(const Square sq) {
            i8 r, f;

            const u8 tr = sq2rank(sq);
            const u8 tf = sq2file(sq);

            board::BitBoard attack_mask = 0ULL;

            for (r = tr + 1; r <= 6; r++)
            {
                attack_mask |= (1ULL << rf2sq(r, tf));
            }
            for (r = tr - 1; r >= 1; r--)
            {
                attack_mask |= (1ULL << rf2sq(r, tf));
            }
            for (f = tf + 1; f <= 6; f++)
            {
                attack_mask |= (1ULL << rf2sq(tr, f));
            }
            for (f = tf - 1; f >= 1; f--)
            {
                attack_mask |= (1ULL << rf2sq(tr, f));
            }

            return attack_mask;
        }

        static board::BitBoard rookAttacks(const Square sq, const board::BitBoard blockers) {
            i8 r, f;

            const u8 tr = sq2rank(sq);
            const u8 tf = sq2file(sq);

            board::BitBoard attack_mask = 0ULL;
            board::BitBoard sqb;

            for (r = tr + 1; r <= 7; r++)
            {
                sqb = 1ULL << rf2sq(r, tf);
                attack_mask |= sqb;
                if (blockers & sqb)
                {
                    break;
                }
            }
            for (r = tr - 1; r >= 0; r--)
            {
                sqb = 1ULL << rf2sq(r, tf);
                attack_mask |= sqb;
                if (blockers & sqb)
                {
                    break;
                }
            }
            for (f = tf + 1; f <= 7; f++)
            {
                sqb = 1ULL << rf2sq(tr, f);
                attack_mask |= sqb;
                if (blockers & sqb)
                {
                    break;
                }
            }
            for (f = tf - 1; f >= 0; f--)
            {
                sqb = 1ULL << rf2sq(tr, f);
                attack_mask |= sqb;
                if (blockers & sqb)
                {
                    break;
                }
            }

            return attack_mask;
        }

        static board::BitBoard getVariant(const u32 index, const u8 bits, board::BitBoard mask) {
            i32             i, j;
            board::BitBoard result = 0ULL;
            for (i = 0; i < bits; i++)
            {
                j = utils::bitScanForward(&mask);
                if (index & (1 << i))
                {
                    result |= (1ULL << j);
                }
            }
            return result;
        }

        static u32 transform(const board::BitBoard b, const board::BitBoard magic, const u8 bits) {
#if defined(USE_32_BIT_MULTIPLICATIONS)
            return (u32) ((i32) b * (i32) magic ^ (i32) (b >> 32) * (i32) (magic >> 32))
                >> (32 - bits);
#else
            return (u32) ((b * magic) >> (64 - bits));
#endif
        }

        static u64 generateMagic() { return utils::prng() & utils::prng() & utils::prng(); }

        static u64 findMagic(const Square sq, const u8 bits, const bool bishop) {
            const board::BitBoard mask = bishop ? bishopMask(sq) : rookMask(sq);
            const u8              n    = utils::bitCount1s(mask);

            board::BitBoard attacks[4096];
            board::BitBoard variants[4096];

            for (i32 i = 0; i < (1 << n); i++)
            {
                variants[i] = getVariant(i, n, mask);
                attacks[i] = bishop ? bishopAttacks(sq, variants[i]) : rookAttacks(sq, variants[i]);
            }

            for (u32 tries = 0; tries < MAGIC_MAX_TRIES; tries++)
            {
                const u64 magic = generateMagic();
                if (utils::bitCount1s((mask * magic) & 0xFF00000000000000ULL) >= 6)
                {
                    continue;
                }

                board::BitBoard used[4096] = {};
                bool            fail       = false;

                for (i32 i = 0; !fail && i < (1 << n); i++)
                {
                    const u32 index = transform(variants[i], magic, bits);
                    if (used[index] == 0ULL)
                    {
                        used[index] = attacks[i];
                    }
                    else if (used[index] != attacks[i])
                    {
                        // Collision
                        fail = true;
                    }
                }
                if (!fail)
                {
                    return magic;
                }
            }

            return 0ULL;
        }

        static void initMagicTableBishop() {
            for (u8 sq = Square::A1; sq <= Square::H8; sq++)
            {
                const Square square         = static_cast<Square>(sq);
                MAGICTABLE_BISHOP[sq].mask  = bishopMask(square);
                MAGICTABLE_BISHOP[sq].shift = utils::bitCount1s(MAGICTABLE_BISHOP[sq].mask);
                MAGICTABLE_BISHOP[sq].magic = findMagic(square, MAGICTABLE_BISHOP[sq].shift, true);
#ifdef DEBUG
                assert(MAGICTABLE_BISHOP[sq].magic != 0ULL);
#endif
            }
        }

        static void initMagicTableRook() {
            for (u8 sq = Square::A1; sq <= Square::H8; sq++)
            {
                const Square square       = static_cast<Square>(sq);
                MAGICTABLE_ROOK[sq].mask  = rookMask(square);
                MAGICTABLE_ROOK[sq].shift = utils::bitCount1s(MAGICTABLE_ROOK[sq].mask);
                MAGICTABLE_ROOK[sq].magic = findMagic(square, MAGICTABLE_ROOK[sq].shift, false);
#ifdef DEBUG
                assert(MAGICTABLE_ROOK[sq].magic != 0ULL);
#endif
            }
        }

        static void initAttackTablePawn() {
            Square          sq;
            board::BitBoard b;
            board::BitBoard attacks;

            // White pawn attacks
            for (u8 r = RANK_1; r <= RANK_8; r++)
            {
                for (u8 f = FILE_A; f <= FILE_H; f++)
                {
                    sq                           = rf2sq(r, f);
                    b                            = 1ULL << sq;
                    attacks                      = board::northEast(b) | board::northWest(b);
                    ATTACK_TABLE_PAWN[WHITE][sq] = attacks;
                }
            }

            // Black pawn attacks
            for (i8 r = RANK_8; r >= RANK_1; r--)
            {
                for (u8 f = FILE_A; f <= FILE_H; f++)
                {
                    sq                           = rf2sq(r, f);
                    b                            = 1ULL << sq;
                    attacks                      = board::southEast(b) | board::southWest(b);
                    ATTACK_TABLE_PAWN[BLACK][sq] = attacks;
                }
            }
        }

        static void initAttackTableKnight() {
            board::BitBoard attacks = 0ULL;
            board::BitBoard b;

            for (u8 sq = 0; sq < 64; sq++)
            {
                b = 1ULL << sq;

                attacks = 0ULL;
                attacks |= (b & board::MASK_NOT_H_FILE) << 17;
                attacks |= (b & board::MASK_NOT_GH_FILE) << 10;
                attacks |= (b & board::MASK_NOT_GH_FILE) >> 6;
                attacks |= (b & board::MASK_NOT_H_FILE) >> 15;
                attacks |= (b & board::MASK_NOT_A_FILE) << 15;
                attacks |= (b & board::MASK_NOT_AB_FILE) << 6;
                attacks |= (b & board::MASK_NOT_AB_FILE) >> 10;
                attacks |= (b & board::MASK_NOT_A_FILE) >> 17;

                ATTACK_TABLE_KNIGHT[sq] = attacks;
            }
        }

        static void initAttackTableBishop() {
            board::BitBoard mask, b;
            u8              n;
            u32             magic_index;

            for (u8 sq = 0; sq < 64; sq++)
            {
                mask = MAGICTABLE_BISHOP[sq].mask;
                n    = MAGICTABLE_BISHOP[sq].shift;

                for (i32 i = 0; i < (1 << n); i++)
                {
                    b           = getVariant(i, n, mask);
                    magic_index = transform(b, MAGICTABLE_BISHOP[sq].magic, n);
                    ATTACK_TABLE_BISHOP[sq][magic_index] =
                      bishopAttacks(static_cast<Square>(sq), b);
                }
            }
        }

        static void initAttackTableRook() {
            board::BitBoard mask, b;
            u8              n;
            u32             magic_index;

            for (u8 sq = 0; sq < 64; sq++)
            {
                mask = MAGICTABLE_ROOK[sq].mask;
                n    = MAGICTABLE_ROOK[sq].shift;

                for (i32 i = 0; i < (1 << n); i++)
                {
                    b                                  = getVariant(i, n, mask);
                    magic_index                        = transform(b, MAGICTABLE_ROOK[sq].magic, n);
                    ATTACK_TABLE_ROOK[sq][magic_index] = rookAttacks(static_cast<Square>(sq), b);
                }
            }
        }

        static void initAttackTableKing() {
            board::BitBoard attacks = 0ULL;
            board::BitBoard b;

            for (u8 sq = 0; sq < 64; sq++)
            {
                b = 1ULL << sq;

                attacks = 0ULL;
                attacks |= board::north(b);
                attacks |= board::south(b);
                attacks |= board::east(b);
                attacks |= board::west(b);
                attacks |= board::northEast(b);
                attacks |= board::southEast(b);
                attacks |= board::southWest(b);
                attacks |= board::northWest(b);

                ATTACK_TABLE_KING[sq] = attacks;
            }
        }

        static board::BitBoard
        getPawnAttacks(const Square sq, const Color c, const board::BitBoard occupancy) {
            return ATTACK_TABLE_PAWN[c][sq] & occupancy;
        }

        static board::BitBoard getKnightAttacks(const Square sq, const board::BitBoard occupancy) {
            return ATTACK_TABLE_KNIGHT[sq] & occupancy;
        }

        static board::BitBoard getBishopAttacks(const Square sq, board::BitBoard occupancy) {
            occupancy = occupancy & MAGICTABLE_BISHOP[sq].mask;
            const u32 index =
              transform(occupancy, MAGICTABLE_BISHOP[sq].magic, MAGICTABLE_BISHOP[sq].shift);
            return ATTACK_TABLE_BISHOP[sq][index];
        }

        static board::BitBoard getRookAttacks(const Square sq, board::BitBoard occupancy) {
            occupancy = occupancy & MAGICTABLE_ROOK[sq].mask;
            const u32 index =
              transform(occupancy, MAGICTABLE_ROOK[sq].magic, MAGICTABLE_ROOK[sq].shift);
            return ATTACK_TABLE_ROOK[sq][index];
        }

        static board::BitBoard getQueenAttacks(const Square sq, board::BitBoard occupancy) {
            return getBishopAttacks(sq, occupancy) | getRookAttacks(sq, occupancy);
        }

        static board::BitBoard getKingAttacks(const Square sq, const board::BitBoard occupancy) {
            return ATTACK_TABLE_KING[sq] & occupancy;
        }

        static void generatePseudolegalMovesPawn(std::vector<move::Move>* moves,
                                                 const board::Board&      board,
                                                 const MovegenType        type) {
            const Color  active_color   = board.getActiveColor();
            const Square ep_target      = board.getEnpassantTarget();
            const Rank   promotion_rank = promotionRankDestOf(active_color);
            const Piece  pawn           = pieceCreate(PieceType::PAWN, active_color);
            auto         bb             = board.getBitboard(pawn);
            while (bb)
            {
                const Square from = static_cast<Square>(utils::bitScanForward(&bb));
                auto         occupancy =
                  board.getBitboard(board::bitboardColorSlot(colorFlip(active_color)));
                if (ep_target != Square::NO_SQ)
                {
                    occupancy |= (1ULL << ep_target);
                }
                auto attacks = getPawnAttacks(from, active_color, occupancy);
                while (attacks)
                {
                    const Square to = static_cast<Square>(utils::bitScanForward(&attacks));
                    if (sq2rank(to) == promotion_rank)
                    {
                        moves->emplace_back(from, to, move::MoveFlag::MOVE_CAPTURE_PROMOTION_QUEEN);
                        moves->emplace_back(from, to, move::MoveFlag::MOVE_CAPTURE_PROMOTION_ROOK);
                        moves->emplace_back(from, to,
                                            move::MoveFlag::MOVE_CAPTURE_PROMOTION_BISHOP);
                        moves->emplace_back(from, to,
                                            move::MoveFlag::MOVE_CAPTURE_PROMOTION_KNIGHT);
                    }
                    else if (to == ep_target)
                    {
                        moves->emplace_back(from, to, move::MoveFlag::MOVE_CAPTURE_EP);
                    }
                    else
                    {
                        moves->emplace_back(from, to, move::MoveFlag::MOVE_CAPTURE);
                    }
                }

                if (type == MovegenType::CAPTURES)
                {
                    continue;
                }
                attacks   = (active_color == Color::WHITE) ? board::north((1ULL << from))
                                                           : board::south((1ULL << from));
                occupancy = board.getBitboard(Piece::NO_PIECE);
                attacks &= occupancy;
                if (attacks)
                {
                    const Square to = static_cast<Square>(utils::bitScanForward(&attacks));
                    if (sq2rank(to) == promotion_rank)
                    {
                        moves->emplace_back(from, to, move::MoveFlag::MOVE_PROMOTION_QUEEN);
                        moves->emplace_back(from, to, move::MoveFlag::MOVE_PROMOTION_ROOK);
                        moves->emplace_back(from, to, move::MoveFlag::MOVE_PROMOTION_BISHOP);
                        moves->emplace_back(from, to, move::MoveFlag::MOVE_PROMOTION_KNIGHT);
                    }
                    else
                    {
                        moves->emplace_back(from, to, move::MoveFlag::MOVE_QUIET);
                    }
                }
                // Pawn Double push
                if (active_color == WHITE)
                {
                    attacks = board::north((1ULL << from)) & occupancy;
                    attacks = board::north(attacks) & occupancy & board::MASK_RANK_4;
                }
                else
                {
                    attacks = board::south((1ULL << from)) & occupancy;
                    attacks = board::south(attacks) & occupancy & board::MASK_RANK_5;
                }
                if (attacks)
                {
                    const Square to = static_cast<Square>(utils::bitScanForward(&attacks));
                    moves->emplace_back(from, to, move::MoveFlag::MOVE_QUIET_PAWN_DBL_PUSH);
                }
            }
        }

        static void generatePseudolegalMovesPiece(std::vector<move::Move>* moves,
                                                  const board::Board&      board,
                                                  const PieceType          piece_type,
                                                  const MovegenType        type) {
            const Color active_color = board.getActiveColor();
            const Piece piece        = pieceCreate(piece_type, active_color);
            auto        bb           = board.getBitboard(piece);
            while (bb)
            {
                const Square from = static_cast<Square>(utils::bitScanForward(&bb));

                board::BitBoard occupancy = 0ULL;
                board::BitBoard attacks   = 0ULL;
                switch (piece_type)
                {
                    case KNIGHT :
                        occupancy =
                          board.getBitboard(board::bitboardColorSlot(colorFlip(active_color)))
                          | board.getBitboard(Piece::NO_PIECE);
                        attacks = getKnightAttacks(from, occupancy);
                        break;

                    case BISHOP :
                        occupancy = ~board.getBitboard(Piece::NO_PIECE);
                        attacks   = getBishopAttacks(from, occupancy);
                        break;

                    case ROOK :
                        occupancy = ~board.getBitboard(Piece::NO_PIECE);
                        attacks   = getRookAttacks(from, occupancy);
                        break;

                    case QUEEN :
                        occupancy = ~board.getBitboard(Piece::NO_PIECE);
                        attacks   = getQueenAttacks(from, occupancy);
                        break;

                    case KING :
                        occupancy =
                          board.getBitboard(board::bitboardColorSlot(colorFlip(active_color)))
                          | board.getBitboard(Piece::NO_PIECE);
                        attacks = getKingAttacks(from, occupancy);
                        break;

                    default :
                        break;
                }

                while (attacks)
                {
                    const Square to       = static_cast<Square>(utils::bitScanForward(&attacks));
                    const Piece  captured = board.getPiece(to);
                    if (type == MovegenType::ALL && captured == Piece::NO_PIECE)
                    {
                        moves->emplace_back(from, to, move::MoveFlag::MOVE_QUIET);
                    }
                    else if (captured != Piece::NO_PIECE && pieceColorOf(captured) != active_color)
                    {
                        moves->emplace_back(from, to, move::MoveFlag::MOVE_CAPTURE);
                    }
                }
            }
        }

        static void generatePseudolegalMovesCastle(std::vector<move::Move>* moves,
                                                   const board::Board&      board) {

            if (isInCheck(board))
            {
                return;
            }

            const Color active_color     = board.getActiveColor();
            const u8    casteling_rights = board.getCastelingRights();

            if (active_color == Color::WHITE)
            {
                if (casteling_rights & board::CastleFlag::WKCA)
                {
                    if (board.getPiece(Square::E1) == Piece::WHITE_KING
                        && board.getPiece(Square::F1) == Piece::NO_PIECE
                        && board.getPiece(Square::G1) == Piece::NO_PIECE
                        && board.getPiece(Square::H1) == Piece::WHITE_ROOK
                        && !isSquareAttacked(board, Square::F1, Color::BLACK))
                    {
                        moves->emplace_back(Square::E1, Square::G1,
                                            move::MoveFlag::MOVE_CASTLE_KING_SIDE);
                    }
                }
                if (casteling_rights & board::CastleFlag::WQCA)
                {
                    if (board.getPiece(Square::E1) == Piece::WHITE_KING
                        && board.getPiece(Square::D1) == Piece::NO_PIECE
                        && board.getPiece(Square::C1) == Piece::NO_PIECE
                        && board.getPiece(Square::B1) == Piece::NO_PIECE
                        && board.getPiece(Square::A1) == Piece::WHITE_ROOK
                        && !isSquareAttacked(board, Square::D1, Color::BLACK))
                    {
                        moves->emplace_back(Square::E1, Square::C1,
                                            move::MoveFlag::MOVE_CASTLE_QUEEN_SIDE);
                    }
                }
            }
            else
            {
                if (casteling_rights & board::CastleFlag::BKCA)
                {
                    if (board.getPiece(Square::E8) == Piece::BLACK_KING
                        && board.getPiece(Square::F8) == Piece::NO_PIECE
                        && board.getPiece(Square::G8) == Piece::NO_PIECE
                        && board.getPiece(Square::H8) == Piece::BLACK_ROOK
                        && !isSquareAttacked(board, Square::F8, Color::WHITE))
                    {
                        moves->emplace_back(Square::E8, Square::G8,
                                            move::MoveFlag::MOVE_CASTLE_KING_SIDE);
                    }
                }
                if (casteling_rights & board::CastleFlag::BQCA)
                {
                    if (board.getPiece(Square::E8) == Piece::BLACK_KING
                        && board.getPiece(Square::D8) == Piece::NO_PIECE
                        && board.getPiece(Square::C8) == Piece::NO_PIECE
                        && board.getPiece(Square::B8) == Piece::NO_PIECE
                        && board.getPiece(Square::A8) == Piece::BLACK_ROOK
                        && !isSquareAttacked(board, Square::D8, Color::WHITE))
                    {
                        moves->emplace_back(Square::E8, Square::C8,
                                            move::MoveFlag::MOVE_CASTLE_QUEEN_SIDE);
                    }
                }
            }
        }

        void initialize() {
            initMagicTableBishop();
            initMagicTableRook();

            initAttackTablePawn();
            initAttackTableKnight();
            initAttackTableBishop();
            initAttackTableRook();
            initAttackTableKing();
        }

        bool isSquareAttacked(const board::Board& board, const Square sq, const Color attacked_by) {
            const board::BitBoard occupancy = ~(board.getBitboard(Piece::NO_PIECE));

            Piece piece = pieceCreate(PieceType::QUEEN, attacked_by);
            if (getQueenAttacks(sq, occupancy) & board.getBitboard(piece))
            {
                return true;
            }

            piece = pieceCreate(PieceType::ROOK, attacked_by);
            if (getRookAttacks(sq, occupancy) & board.getBitboard(piece))
            {
                return true;
            }

            piece = pieceCreate(PieceType::BISHOP, attacked_by);
            if (getBishopAttacks(sq, occupancy) & board.getBitboard(piece))
            {
                return true;
            }

            piece = pieceCreate(PieceType::KNIGHT, attacked_by);
            if (ATTACK_TABLE_KNIGHT[sq] & board.getBitboard(piece))
            {
                return true;
            }

            piece = pieceCreate(PieceType::PAWN, attacked_by);
            if (ATTACK_TABLE_PAWN[colorFlip(attacked_by)][sq] & board.getBitboard(piece))
            {
                return true;
            }

            piece = pieceCreate(PieceType::KING, attacked_by);
            if (ATTACK_TABLE_KING[sq] & board.getBitboard(piece))
            {
                return true;
            }

            return false;
        }

        bool isInCheck(const board::Board& board) {
            const Piece     king        = pieceCreate(PieceType::KING, board.getActiveColor());
            board::BitBoard bb          = board.getBitboard(king);
            const Square    sq          = static_cast<Square>(utils::bitScanForward(&bb));
            const Color     attacked_by = colorFlip(board.getActiveColor());
            return isSquareAttacked(board, sq, attacked_by);
        }


        void generatePseudolegalMoves(std::vector<move::Move>* moves,
                                      const board::Board&      board,
                                      const MovegenType        type) {
            generatePseudolegalMovesPawn(moves, board, type);
            generatePseudolegalMovesPiece(moves, board, PieceType::KNIGHT, type);
            generatePseudolegalMovesPiece(moves, board, PieceType::BISHOP, type);
            generatePseudolegalMovesPiece(moves, board, PieceType::ROOK, type);
            generatePseudolegalMovesPiece(moves, board, PieceType::QUEEN, type);
            generatePseudolegalMovesPiece(moves, board, PieceType::KING, type);
            generatePseudolegalMovesCastle(moves, board);
        }

    }

}
