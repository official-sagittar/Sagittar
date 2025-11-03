#include "movegen.h"
#include "commons/utils.h"

namespace sagittar {

    static constexpr unsigned int MAGIC_MAX_TRIES = 500000;

    struct Magic {
        BitBoard mask;
        u64      magic;
        u8       shift;
    };

    static Magic MAGICTABLE_BISHOP[64];
    static Magic MAGICTABLE_ROOK[64];

    static BitBoard ATTACK_TABLE_PAWN[2][64];
    static BitBoard ATTACK_TABLE_KNIGHT[64];
    static BitBoard ATTACK_TABLE_BISHOP[64][512];
    static BitBoard ATTACK_TABLE_ROOK[64][4096];
    static BitBoard ATTACK_TABLE_KING[64];

    static std::vector<std::function<BitBoard(const Square, BitBoard)>> attackFunctions;

    template<Color US>
    static constexpr int BITBOARD_FWD_DIR = (US == WHITE) ? 8 : -8;
    template<Color US>
    static constexpr int BITBOARD_FWD_DBL_DIR = (US == WHITE) ? 16 : -16;
    template<Color US>
    static constexpr int BITBOARD_CAPTURE_LEFT_DIR = (US == WHITE) ? 7 : -7;
    template<Color US>
    static constexpr int BITBOARD_CAPTURE_RIGHT_DIR = (US == WHITE) ? 9 : -9;

    static BitBoard bishopMask(const Square sq) {
        i8 r, f;

        const u8 tr = sq2rank(sq);
        const u8 tf = sq2file(sq);

        BitBoard attack_mask = 0ULL;

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

    static BitBoard bishopAttacks(const Square sq, const BitBoard blockers) {
        i8 r, f;

        const u8 tr = sq2rank(sq);
        const u8 tf = sq2file(sq);

        BitBoard attack_mask = 0ULL;
        BitBoard sqb;

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

    static BitBoard rookMask(const Square sq) {
        i8 r, f;

        const u8 tr = sq2rank(sq);
        const u8 tf = sq2file(sq);

        BitBoard attack_mask = 0ULL;

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

    static BitBoard rookAttacks(const Square sq, const BitBoard blockers) {
        i8 r, f;

        const u8 tr = sq2rank(sq);
        const u8 tf = sq2file(sq);

        BitBoard attack_mask = 0ULL;
        BitBoard sqb;

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

    static BitBoard getVariant(const u32 index, const u8 bits, BitBoard mask) {
        i32      i, j;
        BitBoard result = 0ULL;
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

    static u32 transform(const BitBoard b, const BitBoard magic, const u8 bits) {
#if defined(USE_32_BIT_MULTIPLICATIONS)
        return (u32) ((i32) b * (i32) magic ^ (i32) (b >> 32) * (i32) (magic >> 32)) >> (32 - bits);
#else
        return (u32) ((b * magic) >> (64 - bits));
#endif
    }

    static u64 generateMagic() { return utils::prng() & utils::prng() & utils::prng(); }

    static u64 findMagic(const Square sq, const u8 bits, const bool bishop) {
        const BitBoard mask = bishop ? bishopMask(sq) : rookMask(sq);
        const u8       n    = utils::bitCount1s(mask);

        BitBoard attacks[4096];
        BitBoard variants[4096];

        for (i32 i = 0; i < (1 << n); i++)
        {
            variants[i] = getVariant(i, n, mask);
            attacks[i]  = bishop ? bishopAttacks(sq, variants[i]) : rookAttacks(sq, variants[i]);
        }

        for (u32 tries = 0; tries < MAGIC_MAX_TRIES; tries++)
        {
            const u64 magic = generateMagic();
            if (utils::bitCount1s((mask * magic) & 0xFF00000000000000ULL) >= 6)
            {
                continue;
            }

            BitBoard used[4096] = {};
            bool     fail       = false;

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
            assert(MAGICTABLE_BISHOP[sq].magic != 0ULL);
        }
    }

    static void initMagicTableRook() {
        for (u8 sq = Square::A1; sq <= Square::H8; sq++)
        {
            const Square square       = static_cast<Square>(sq);
            MAGICTABLE_ROOK[sq].mask  = rookMask(square);
            MAGICTABLE_ROOK[sq].shift = utils::bitCount1s(MAGICTABLE_ROOK[sq].mask);
            MAGICTABLE_ROOK[sq].magic = findMagic(square, MAGICTABLE_ROOK[sq].shift, false);
            assert(MAGICTABLE_ROOK[sq].magic != 0ULL);
        }
    }

    static void initAttackTablePawn() {
        Square   sq;
        BitBoard b;
        BitBoard attacks;

        // White pawn attacks
        for (u8 r = RANK_1; r <= RANK_8; r++)
        {
            for (u8 f = FILE_A; f <= FILE_H; f++)
            {
                sq                           = rf2sq(r, f);
                b                            = 1ULL << sq;
                attacks                      = northEast(b) | northWest(b);
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
                attacks                      = southEast(b) | southWest(b);
                ATTACK_TABLE_PAWN[BLACK][sq] = attacks;
            }
        }
    }

    static void initAttackTableKnight() {
        BitBoard attacks = 0ULL;
        BitBoard b;

        for (u8 sq = 0; sq < 64; sq++)
        {
            b = 1ULL << sq;

            attacks = 0ULL;
            attacks |= (b & MASK_NOT_H_FILE) << 17;
            attacks |= (b & MASK_NOT_GH_FILE) << 10;
            attacks |= (b & MASK_NOT_GH_FILE) >> 6;
            attacks |= (b & MASK_NOT_H_FILE) >> 15;
            attacks |= (b & MASK_NOT_A_FILE) << 15;
            attacks |= (b & MASK_NOT_AB_FILE) << 6;
            attacks |= (b & MASK_NOT_AB_FILE) >> 10;
            attacks |= (b & MASK_NOT_A_FILE) >> 17;

            ATTACK_TABLE_KNIGHT[sq] = attacks;
        }
    }

    static void initAttackTableBishop() {
        BitBoard mask, b;
        u8       n;
        u32      magic_index;

        for (u8 sq = 0; sq < 64; sq++)
        {
            mask = MAGICTABLE_BISHOP[sq].mask;
            n    = MAGICTABLE_BISHOP[sq].shift;

            for (i32 i = 0; i < (1 << n); i++)
            {
                b                                    = getVariant(i, n, mask);
                magic_index                          = transform(b, MAGICTABLE_BISHOP[sq].magic, n);
                ATTACK_TABLE_BISHOP[sq][magic_index] = bishopAttacks(static_cast<Square>(sq), b);
            }
        }
    }

    static void initAttackTableRook() {
        BitBoard mask, b;
        u8       n;
        u32      magic_index;

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
        BitBoard attacks = 0ULL;
        BitBoard b;

        for (u8 sq = 0; sq < 64; sq++)
        {
            b = 1ULL << sq;

            attacks = 0ULL;
            attacks |= north(b);
            attacks |= south(b);
            attacks |= east(b);
            attacks |= west(b);
            attacks |= northEast(b);
            attacks |= southEast(b);
            attacks |= southWest(b);
            attacks |= northWest(b);

            ATTACK_TABLE_KING[sq] = attacks;
        }
    }

    static BitBoard getPawnAttacks(const Square sq, const Color c, const BitBoard occupancy) {
        return ATTACK_TABLE_PAWN[c][sq] & occupancy;
    }

    static BitBoard getKnightAttacks(const Square sq, const BitBoard occupancy) {
        return ATTACK_TABLE_KNIGHT[sq] & occupancy;
    }

    static BitBoard getBishopAttacks(const Square sq, BitBoard occupancy) {
        occupancy = occupancy & MAGICTABLE_BISHOP[sq].mask;
        const u32 index =
          transform(occupancy, MAGICTABLE_BISHOP[sq].magic, MAGICTABLE_BISHOP[sq].shift);
        return ATTACK_TABLE_BISHOP[sq][index];
    }

    static BitBoard getRookAttacks(const Square sq, BitBoard occupancy) {
        occupancy = occupancy & MAGICTABLE_ROOK[sq].mask;
        const u32 index =
          transform(occupancy, MAGICTABLE_ROOK[sq].magic, MAGICTABLE_ROOK[sq].shift);
        return ATTACK_TABLE_ROOK[sq][index];
    }

    static BitBoard getQueenAttacks(const Square sq, BitBoard occupancy) {
        return getBishopAttacks(sq, occupancy) | getRookAttacks(sq, occupancy);
    }

    static BitBoard getKingAttacks(const Square sq, const BitBoard occupancy) {
        return ATTACK_TABLE_KING[sq] & occupancy;
    }

    template<Color US, MovegenType T>
    static void generatePseudolegalMovesPawn(containers::ArrayList<Move>* moves,
                                             const Position&              pos) {
        constexpr Color    them           = colorFlip(US);
        constexpr BitBoard promo_dest     = (US == Color::WHITE) ? MASK_RANK_8 : MASK_RANK_1;
        constexpr BitBoard not_promo_dest = ~promo_dest;
        constexpr BitBoard ep_target_rank = (US == Color::WHITE) ? MASK_RANK_6 : MASK_RANK_3;

        const BitBoard pawns     = pos.pieces(US, PieceType::PAWN);
        const BitBoard king_them = pos.pieces(them, PieceType::KING);
        const BitBoard enemies   = pos.pieces(them) ^ king_them;
        const BitBoard empty     = pos.empty();

        BitBoard pawns_fwd, sgl_push, dbl_push, fwd_l, fwd_r;

        if constexpr (US == Color::WHITE)
        {
            pawns_fwd = north(pawns);
            sgl_push  = pawns_fwd & empty & not_promo_dest;
            dbl_push  = north(sgl_push) & MASK_RANK_4 & empty;
            fwd_l     = northWest(pawns);
            fwd_r     = northEast(pawns);
        }
        else
        {
            pawns_fwd = south(pawns);
            sgl_push  = pawns_fwd & empty & not_promo_dest;
            dbl_push  = south(sgl_push) & MASK_RANK_5 & empty;
            fwd_l     = southEast(pawns);
            fwd_r     = southWest(pawns);
        }

        const BitBoard enemies_not_on_promotion_dest = enemies & not_promo_dest;
        const BitBoard capture_l                     = fwd_l & enemies_not_on_promotion_dest;
        const BitBoard capture_r                     = fwd_r & enemies_not_on_promotion_dest;
        const BitBoard ep_target_bb =
          (pos.epTarget() != Square::NO_SQ) ? ((1ULL << pos.epTarget()) & ep_target_rank) : 0ULL;
        const BitBoard capture_ep_l              = fwd_l & ep_target_bb;
        const BitBoard capture_ep_r              = fwd_r & ep_target_bb;
        const BitBoard quite_promo               = pawns_fwd & promo_dest & empty;
        const BitBoard enemies_on_promotion_dest = enemies & promo_dest;
        const BitBoard capture_promo_l           = fwd_l & enemies_on_promotion_dest;
        const BitBoard capture_promo_r           = fwd_r & enemies_on_promotion_dest;

        BitBoard bb;
        int      dir;

        bb  = capture_promo_l;
        dir = BITBOARD_CAPTURE_LEFT_DIR<US>;
        while (bb)
        {
            const Square to   = static_cast<Square>(utils::bitScanForward(&bb));
            const Square from = static_cast<Square>(to - dir);
            moves->emplace_back(from, to, MoveFlag::MOVE_CAPTURE_PROMOTION_QUEEN);
            moves->emplace_back(from, to, MoveFlag::MOVE_CAPTURE_PROMOTION_ROOK);
            moves->emplace_back(from, to, MoveFlag::MOVE_CAPTURE_PROMOTION_BISHOP);
            moves->emplace_back(from, to, MoveFlag::MOVE_CAPTURE_PROMOTION_KNIGHT);
        }

        bb  = capture_promo_r;
        dir = BITBOARD_CAPTURE_RIGHT_DIR<US>;
        while (bb)
        {
            const Square to   = static_cast<Square>(utils::bitScanForward(&bb));
            const Square from = static_cast<Square>(to - dir);
            moves->emplace_back(from, to, MoveFlag::MOVE_CAPTURE_PROMOTION_QUEEN);
            moves->emplace_back(from, to, MoveFlag::MOVE_CAPTURE_PROMOTION_ROOK);
            moves->emplace_back(from, to, MoveFlag::MOVE_CAPTURE_PROMOTION_BISHOP);
            moves->emplace_back(from, to, MoveFlag::MOVE_CAPTURE_PROMOTION_KNIGHT);
        }

        dir = BITBOARD_CAPTURE_LEFT_DIR<US>;

        bb = capture_l;
        while (bb)
        {
            const Square to   = static_cast<Square>(utils::bitScanForward(&bb));
            const Square from = static_cast<Square>(to - dir);
            moves->emplace_back(from, to, MoveFlag::MOVE_CAPTURE);
        }

        bb = capture_ep_l;
        while (bb)
        {
            const Square to   = static_cast<Square>(utils::bitScanForward(&bb));
            const Square from = static_cast<Square>(to - dir);
            moves->emplace_back(from, to, MoveFlag::MOVE_CAPTURE_EP);
        }

        dir = BITBOARD_CAPTURE_RIGHT_DIR<US>;

        bb = capture_r;
        while (bb)
        {
            const Square to   = static_cast<Square>(utils::bitScanForward(&bb));
            const Square from = static_cast<Square>(to - dir);
            moves->emplace_back(from, to, MoveFlag::MOVE_CAPTURE);
        }

        bb = capture_ep_r;
        while (bb)
        {
            const Square to   = static_cast<Square>(utils::bitScanForward(&bb));
            const Square from = static_cast<Square>(to - dir);
            moves->emplace_back(from, to, MoveFlag::MOVE_CAPTURE_EP);
        }

        if constexpr (T == MovegenType::ALL)
        {
            dir = BITBOARD_FWD_DIR<US>;

            bb = quite_promo;
            while (bb)
            {
                const Square to   = static_cast<Square>(utils::bitScanForward(&bb));
                const Square from = static_cast<Square>(to - dir);
                moves->emplace_back(from, to, MoveFlag::MOVE_PROMOTION_QUEEN);
                moves->emplace_back(from, to, MoveFlag::MOVE_PROMOTION_ROOK);
                moves->emplace_back(from, to, MoveFlag::MOVE_PROMOTION_BISHOP);
                moves->emplace_back(from, to, MoveFlag::MOVE_PROMOTION_KNIGHT);
            }

            bb = sgl_push;
            while (bb)
            {
                const Square to   = static_cast<Square>(utils::bitScanForward(&bb));
                const Square from = static_cast<Square>(to - dir);
                moves->emplace_back(from, to, MoveFlag::MOVE_QUIET);
            }

            bb  = dbl_push;
            dir = BITBOARD_FWD_DBL_DIR<US>;
            while (bb)
            {
                const Square to   = static_cast<Square>(utils::bitScanForward(&bb));
                const Square from = static_cast<Square>(to - dir);
                moves->emplace_back(from, to, MoveFlag::MOVE_QUIET_PAWN_DBL_PUSH);
            }
        }
    }

    template<PieceType PT, Color US, MovegenType T>
    static void generatePseudolegalMovesPiece(containers::ArrayList<Move>* moves,
                                              const Position&              pos) {
        constexpr Color them      = colorFlip(US);
        const BitBoard  king_them = pos.pieces(them, PieceType::KING);
        const BitBoard  enemies   = pos.pieces(them) ^ king_them;
        const BitBoard  occupied  = pos.occupied();
        const BitBoard  empty     = ~occupied;

        BitBoard occupancy = occupied;
        if constexpr ((PT == PieceType::KNIGHT) || (PT == PieceType::KING))
        {
            occupancy = (enemies | empty);
        }

        BitBoard bb = pos.pieces(US, PT);
        while (bb)
        {
            const Square from = static_cast<Square>(utils::bitScanForward(&bb));

            BitBoard attacks = 0ULL;
            if constexpr (PT == PieceType::KNIGHT)
            {
                attacks = ATTACK_TABLE_KNIGHT[from] & occupancy;
            }
            else if constexpr (PT == PieceType::BISHOP)
            {
                attacks = getBishopAttacks(from, occupancy);
            }
            else if constexpr (PT == PieceType::ROOK)
            {
                attacks = getRookAttacks(from, occupancy);
            }
            else if constexpr (PT == PieceType::QUEEN)
            {
                attacks = getBishopAttacks(from, occupancy) | getRookAttacks(from, occupancy);
            }
            else if constexpr (PT == PieceType::KING)
            {
                attacks = ATTACK_TABLE_KING[from] & occupancy;
            }

            BitBoard captures = attacks & enemies;
            while (captures)
            {
                const Square to = static_cast<Square>(utils::bitScanForward(&captures));
                moves->emplace_back(from, to, MoveFlag::MOVE_CAPTURE);
            }

            if constexpr (T == MovegenType::ALL)
            {
                BitBoard quites = attacks & empty;
                while (quites)
                {
                    const Square to = static_cast<Square>(utils::bitScanForward(&quites));
                    moves->emplace_back(from, to, MoveFlag::MOVE_QUIET);
                }
            }
        }
    }

    static void generatePseudolegalMovesCastle(containers::ArrayList<Move>* moves,
                                               const Position&              pos) {

        if (pos.isInCheck())
        {
            return;
        }

        const Color active_color     = pos.stm();
        const u8    casteling_rights = pos.caRights();

        if (active_color == Color::WHITE)
        {
            if (casteling_rights & CastleFlag::WKCA)
            {
                if (pos.pieceOn(Square::E1) == Piece::WHITE_KING
                    && pos.pieceOn(Square::F1) == Piece::NO_PIECE
                    && pos.pieceOn(Square::G1) == Piece::NO_PIECE
                    && pos.pieceOn(Square::H1) == Piece::WHITE_ROOK
                    && !getSquareAttackers(pos, Square::F1, Color::BLACK))
                {
                    moves->emplace_back(Square::E1, Square::G1, MoveFlag::MOVE_CASTLE_KING_SIDE);
                }
            }
            if (casteling_rights & CastleFlag::WQCA)
            {
                if (pos.pieceOn(Square::E1) == Piece::WHITE_KING
                    && pos.pieceOn(Square::D1) == Piece::NO_PIECE
                    && pos.pieceOn(Square::C1) == Piece::NO_PIECE
                    && pos.pieceOn(Square::B1) == Piece::NO_PIECE
                    && pos.pieceOn(Square::A1) == Piece::WHITE_ROOK
                    && !getSquareAttackers(pos, Square::D1, Color::BLACK))
                {
                    moves->emplace_back(Square::E1, Square::C1, MoveFlag::MOVE_CASTLE_QUEEN_SIDE);
                }
            }
        }
        else
        {
            if (casteling_rights & CastleFlag::BKCA)
            {
                if (pos.pieceOn(Square::E8) == Piece::BLACK_KING
                    && pos.pieceOn(Square::F8) == Piece::NO_PIECE
                    && pos.pieceOn(Square::G8) == Piece::NO_PIECE
                    && pos.pieceOn(Square::H8) == Piece::BLACK_ROOK
                    && !getSquareAttackers(pos, Square::F8, Color::WHITE))
                {
                    moves->emplace_back(Square::E8, Square::G8, MoveFlag::MOVE_CASTLE_KING_SIDE);
                }
            }
            if (casteling_rights & CastleFlag::BQCA)
            {
                if (pos.pieceOn(Square::E8) == Piece::BLACK_KING
                    && pos.pieceOn(Square::D8) == Piece::NO_PIECE
                    && pos.pieceOn(Square::C8) == Piece::NO_PIECE
                    && pos.pieceOn(Square::B8) == Piece::NO_PIECE
                    && pos.pieceOn(Square::A8) == Piece::BLACK_ROOK
                    && !getSquareAttackers(pos, Square::D8, Color::WHITE))
                {
                    moves->emplace_back(Square::E8, Square::C8, MoveFlag::MOVE_CASTLE_QUEEN_SIDE);
                }
            }
        }
    }

    void movegen_initialize() {
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

    BitBoard getSquareAttackers(const Position& pos, const Square sq, const Color attacked_by) {
        const BitBoard occupied = pos.occupied();
        BitBoard       opPawns, opKnights, opRQ, opBQ, opKing;
        opPawns   = pos.pieces(attacked_by, PieceType::PAWN);
        opKnights = pos.pieces(attacked_by, PieceType::KNIGHT);
        opRQ = opBQ = pos.pieces(attacked_by, PieceType::QUEEN);
        opRQ |= pos.pieces(attacked_by, PieceType::ROOK);
        opBQ |= pos.pieces(attacked_by, PieceType::BISHOP);
        opKing = pos.pieces(attacked_by, PieceType::KING);
        // clang-format off
            return (getBishopAttacks(sq, occupied) & opBQ)
                 | (getRookAttacks(sq, occupied) & opRQ)
                 | (ATTACK_TABLE_KNIGHT[sq] & opKnights)
                 | (ATTACK_TABLE_PAWN[colorFlip(attacked_by)][sq] & opPawns)
                 | (ATTACK_TABLE_KING[sq] & opKing);
        // clang-format on
    }

    template<MovegenType T>
    void generatePseudolegalMoves(containers::ArrayList<Move>* moves, const Position& pos) {
        if (pos.stm() == Color::WHITE)
        {
            generatePseudolegalMovesPawn<Color::WHITE, T>(moves, pos);
            generatePseudolegalMovesPiece<PieceType::QUEEN, Color::WHITE, T>(moves, pos);
            generatePseudolegalMovesPiece<PieceType::ROOK, Color::WHITE, T>(moves, pos);
            generatePseudolegalMovesPiece<PieceType::BISHOP, Color::WHITE, T>(moves, pos);
            generatePseudolegalMovesPiece<PieceType::KNIGHT, Color::WHITE, T>(moves, pos);
            generatePseudolegalMovesPiece<PieceType::KING, Color::WHITE, T>(moves, pos);
        }
        else
        {
            generatePseudolegalMovesPawn<Color::BLACK, T>(moves, pos);
            generatePseudolegalMovesPiece<PieceType::QUEEN, Color::BLACK, T>(moves, pos);
            generatePseudolegalMovesPiece<PieceType::ROOK, Color::BLACK, T>(moves, pos);
            generatePseudolegalMovesPiece<PieceType::BISHOP, Color::BLACK, T>(moves, pos);
            generatePseudolegalMovesPiece<PieceType::KNIGHT, Color::BLACK, T>(moves, pos);
            generatePseudolegalMovesPiece<PieceType::KING, Color::BLACK, T>(moves, pos);
        }
        if constexpr (T == MovegenType::ALL)
        {
            generatePseudolegalMovesCastle(moves, pos);
        }
    }

    template void generatePseudolegalMoves<MovegenType::ALL>(containers::ArrayList<Move>* moves,
                                                             const Position&              pos);
    template void
    generatePseudolegalMoves<MovegenType::CAPTURES>(containers::ArrayList<Move>* moves,
                                                    const Position&              pos);

}
