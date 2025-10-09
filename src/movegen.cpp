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

        static std::vector<std::function<board::BitBoard(const Square, board::BitBoard)>>
          attackFunctions;

        template<Color US>
        static constexpr int BITBOARD_FWD_DIR = (US == WHITE) ? 8 : -8;
        template<Color US>
        static constexpr int BITBOARD_FWD_DBL_DIR = (US == WHITE) ? 16 : -16;
        template<Color US>
        static constexpr int BITBOARD_CAPTURE_LEFT_DIR = (US == WHITE) ? 7 : -7;
        template<Color US>
        static constexpr int BITBOARD_CAPTURE_RIGHT_DIR = (US == WHITE) ? 9 : -9;

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

        template<Color US, MovegenType T>
        static void generatePseudolegalMovesPawn(containers::ArrayList<move::Move>* moves,
                                                 const board::Board&                board) {
            constexpr Color           them = colorFlip(US);
            constexpr board::BitBoard promo_dest =
              (US == Color::WHITE) ? board::MASK_RANK_8 : board::MASK_RANK_1;
            constexpr board::BitBoard not_promo_dest = ~promo_dest;
            constexpr board::BitBoard ep_target_rank =
              (US == Color::WHITE) ? board::MASK_RANK_6 : board::MASK_RANK_3;

            const board::BitBoard pawns   = board.getBitboard(PieceType::PAWN, US);
            const board::BitBoard enemies = board.getBitboard(board::bitboardColorSlot(them));
            const board::BitBoard empty   = board.getBitboard(Piece::NO_PIECE);

            board::BitBoard pawns_fwd, sgl_push, dbl_push, fwd_l, fwd_r;

            if constexpr (US == Color::WHITE)
            {
                pawns_fwd = board::north(pawns);
                sgl_push  = pawns_fwd & empty & not_promo_dest;
                dbl_push  = board::north(sgl_push) & board::MASK_RANK_4 & empty;
                fwd_l     = board::northWest(pawns);
                fwd_r     = board::northEast(pawns);
            }
            else
            {
                pawns_fwd = board::south(pawns);
                sgl_push  = pawns_fwd & empty & not_promo_dest;
                dbl_push  = board::south(sgl_push) & board::MASK_RANK_5 & empty;
                fwd_l     = board::southEast(pawns);
                fwd_r     = board::southWest(pawns);
            }

            const board::BitBoard enemies_not_on_promotion_dest = enemies & not_promo_dest;
            const board::BitBoard capture_l = fwd_l & enemies_not_on_promotion_dest;
            const board::BitBoard capture_r = fwd_r & enemies_not_on_promotion_dest;
            const board::BitBoard ep_target_bb =
              (board.getEnpassantTarget() != Square::NO_SQ)
                ? ((1ULL << board.getEnpassantTarget()) & ep_target_rank)
                : 0ULL;
            const board::BitBoard capture_ep_l              = fwd_l & ep_target_bb;
            const board::BitBoard capture_ep_r              = fwd_r & ep_target_bb;
            const board::BitBoard quite_promo               = pawns_fwd & promo_dest & empty;
            const board::BitBoard enemies_on_promotion_dest = enemies & promo_dest;
            const board::BitBoard capture_promo_l           = fwd_l & enemies_on_promotion_dest;
            const board::BitBoard capture_promo_r           = fwd_r & enemies_on_promotion_dest;

            board::BitBoard bb;
            int             dir;

            bb  = capture_promo_l;
            dir = BITBOARD_CAPTURE_LEFT_DIR<US>;
            while (bb)
            {
                const Square to   = static_cast<Square>(utils::bitScanForward(&bb));
                const Square from = static_cast<Square>(to - dir);
                moves->emplace_back(from, to, move::MoveFlag::MOVE_CAPTURE_PROMOTION_QUEEN);
                moves->emplace_back(from, to, move::MoveFlag::MOVE_CAPTURE_PROMOTION_ROOK);
                moves->emplace_back(from, to, move::MoveFlag::MOVE_CAPTURE_PROMOTION_BISHOP);
                moves->emplace_back(from, to, move::MoveFlag::MOVE_CAPTURE_PROMOTION_KNIGHT);
            }

            bb  = capture_promo_r;
            dir = BITBOARD_CAPTURE_RIGHT_DIR<US>;
            while (bb)
            {
                const Square to   = static_cast<Square>(utils::bitScanForward(&bb));
                const Square from = static_cast<Square>(to - dir);
                moves->emplace_back(from, to, move::MoveFlag::MOVE_CAPTURE_PROMOTION_QUEEN);
                moves->emplace_back(from, to, move::MoveFlag::MOVE_CAPTURE_PROMOTION_ROOK);
                moves->emplace_back(from, to, move::MoveFlag::MOVE_CAPTURE_PROMOTION_BISHOP);
                moves->emplace_back(from, to, move::MoveFlag::MOVE_CAPTURE_PROMOTION_KNIGHT);
            }

            dir = BITBOARD_CAPTURE_LEFT_DIR<US>;

            bb = capture_l;
            while (bb)
            {
                const Square to   = static_cast<Square>(utils::bitScanForward(&bb));
                const Square from = static_cast<Square>(to - dir);
                moves->emplace_back(from, to, move::MoveFlag::MOVE_CAPTURE);
            }

            bb = capture_ep_l;
            while (bb)
            {
                const Square to   = static_cast<Square>(utils::bitScanForward(&bb));
                const Square from = static_cast<Square>(to - dir);
                moves->emplace_back(from, to, move::MoveFlag::MOVE_CAPTURE_EP);
            }

            dir = BITBOARD_CAPTURE_RIGHT_DIR<US>;

            bb = capture_r;
            while (bb)
            {
                const Square to   = static_cast<Square>(utils::bitScanForward(&bb));
                const Square from = static_cast<Square>(to - dir);
                moves->emplace_back(from, to, move::MoveFlag::MOVE_CAPTURE);
            }

            bb = capture_ep_r;
            while (bb)
            {
                const Square to   = static_cast<Square>(utils::bitScanForward(&bb));
                const Square from = static_cast<Square>(to - dir);
                moves->emplace_back(from, to, move::MoveFlag::MOVE_CAPTURE_EP);
            }

            if constexpr (T == MovegenType::ALL)
            {
                dir = BITBOARD_FWD_DIR<US>;

                bb = quite_promo;
                while (bb)
                {
                    const Square to   = static_cast<Square>(utils::bitScanForward(&bb));
                    const Square from = static_cast<Square>(to - dir);
                    moves->emplace_back(from, to, move::MoveFlag::MOVE_PROMOTION_QUEEN);
                    moves->emplace_back(from, to, move::MoveFlag::MOVE_PROMOTION_ROOK);
                    moves->emplace_back(from, to, move::MoveFlag::MOVE_PROMOTION_BISHOP);
                    moves->emplace_back(from, to, move::MoveFlag::MOVE_PROMOTION_KNIGHT);
                }

                bb = sgl_push;
                while (bb)
                {
                    const Square to   = static_cast<Square>(utils::bitScanForward(&bb));
                    const Square from = static_cast<Square>(to - dir);
                    moves->emplace_back(from, to, move::MoveFlag::MOVE_QUIET);
                }

                bb  = dbl_push;
                dir = BITBOARD_FWD_DBL_DIR<US>;
                while (bb)
                {
                    const Square to   = static_cast<Square>(utils::bitScanForward(&bb));
                    const Square from = static_cast<Square>(to - dir);
                    moves->emplace_back(from, to, move::MoveFlag::MOVE_QUIET_PAWN_DBL_PUSH);
                }
            }
        }

        template<PieceType PieceTypeName>
        static void generatePseudolegalMovesPiece(containers::ArrayList<move::Move>* moves,
                                                  const board::Board&                board,
                                                  const MovegenType                  type) {
            const Color     active_color = board.getActiveColor();
            const Piece     piece        = pieceCreate(PieceTypeName, active_color);
            board::BitBoard bb           = board.getBitboard(piece);
            board::BitBoard occupancy    = 0ULL;
            const auto      attackFn     = attackFunctions.at(PieceTypeName - 2);

            switch (PieceTypeName)
            {
                case PieceType::KNIGHT :
                case PieceType::KING :
                    occupancy = board.getBitboard(board::bitboardColorSlot(colorFlip(active_color)))
                              | board.getBitboard(Piece::NO_PIECE);
                    break;

                case PieceType::BISHOP :
                case PieceType::ROOK :
                case PieceType::QUEEN :
                    occupancy = ~board.getBitboard(Piece::NO_PIECE);
                    break;
            }

            while (bb)
            {
                const Square    from    = static_cast<Square>(utils::bitScanForward(&bb));
                board::BitBoard attacks = attackFn(from, occupancy);
                while (attacks)
                {
                    const Square to       = static_cast<Square>(utils::bitScanForward(&attacks));
                    const Piece  captured = board.getPiece(to);
                    switch (type)
                    {
                        case MovegenType::ALL :
                            if (captured == Piece::NO_PIECE)
                            {
                                moves->emplace_back(from, to, move::MoveFlag::MOVE_QUIET);
                            }
                            [[fallthrough]];
                        case MovegenType::CAPTURES :
                            if (captured != Piece::NO_PIECE
                                && pieceColorOf(captured) != active_color)
                            {
                                moves->emplace_back(from, to, move::MoveFlag::MOVE_CAPTURE);
                            }
                            break;
                    }
                }
            }
        }

        static void generatePseudolegalMovesCastle(containers::ArrayList<move::Move>* moves,
                                                   const board::Board&                board) {

            if (board.isInCheck())
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
                        && !getSquareAttackers(board, Square::F1, Color::BLACK))
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
                        && !getSquareAttackers(board, Square::D1, Color::BLACK))
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
                        && !getSquareAttackers(board, Square::F8, Color::WHITE))
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
                        && !getSquareAttackers(board, Square::D8, Color::WHITE))
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

            attackFunctions.push_back(getKnightAttacks);
            attackFunctions.push_back(getBishopAttacks);
            attackFunctions.push_back(getRookAttacks);
            attackFunctions.push_back(getQueenAttacks);
            attackFunctions.push_back(getKingAttacks);
        }

        board::BitBoard
        getSquareAttackers(const board::Board& board, const Square sq, const Color attacked_by) {
            const board::BitBoard occupied = ~(board.getBitboard(Piece::NO_PIECE));
            board::BitBoard       opPawns, opKnights, opRQ, opBQ, opKing;
            opPawns   = board.getBitboard(pieceCreate(PieceType::PAWN, attacked_by));
            opKnights = board.getBitboard(pieceCreate(PieceType::KNIGHT, attacked_by));
            opRQ = opBQ = board.getBitboard(pieceCreate(PieceType::QUEEN, attacked_by));
            opRQ |= board.getBitboard(pieceCreate(PieceType::ROOK, attacked_by));
            opBQ |= board.getBitboard(pieceCreate(PieceType::BISHOP, attacked_by));
            opKing = board.getBitboard(pieceCreate(PieceType::KING, attacked_by));
            // clang-format off
            return (getBishopAttacks(sq, occupied) & opBQ)
                 | (getRookAttacks(sq, occupied) & opRQ)
                 | (ATTACK_TABLE_KNIGHT[sq] & opKnights)
                 | (ATTACK_TABLE_PAWN[colorFlip(attacked_by)][sq] & opPawns)
                 | (ATTACK_TABLE_KING[sq] & opKing);
            // clang-format on
        }

        template<MovegenType T>
        void generatePseudolegalMoves(containers::ArrayList<move::Move>* moves,
                                      const board::Board&                board) {
            if (board.getActiveColor() == Color::WHITE)
            {
                generatePseudolegalMovesPawn<WHITE, T>(moves, board);
            }
            else
            {
                generatePseudolegalMovesPawn<BLACK, T>(moves, board);
            }
            generatePseudolegalMovesPiece<PieceType::KNIGHT>(moves, board, T);
            generatePseudolegalMovesPiece<PieceType::BISHOP>(moves, board, T);
            generatePseudolegalMovesPiece<PieceType::ROOK>(moves, board, T);
            generatePseudolegalMovesPiece<PieceType::QUEEN>(moves, board, T);
            generatePseudolegalMovesPiece<PieceType::KING>(moves, board, T);
            if constexpr (T == MovegenType::ALL)
            {
                generatePseudolegalMovesCastle(moves, board);
            }
        }

        template void
        generatePseudolegalMoves<MovegenType::ALL>(containers::ArrayList<move::Move>* moves,
                                                   const board::Board&                board);
        template void
        generatePseudolegalMoves<MovegenType::CAPTURES>(containers::ArrayList<move::Move>* moves,
                                                        const board::Board&                board);

    }

}
