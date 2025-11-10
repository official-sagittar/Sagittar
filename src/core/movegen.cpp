#include "movegen.h"
#include "commons/utils.h"

namespace sagittar {

    struct Magic {
        BitBoard mask;
        u64      magic;
        u8       shift;

        unsigned index(BitBoard occ) const {
#if defined(IS_32_BIT)
            const u32 lo = unsigned(occ) & unsigned(mask);
            const u32 hi = unsigned(occ >> 32) & unsigned(mask >> 32);
            return ((lo * unsigned(magic)) ^ (hi * unsigned(magic >> 32))) >> shift;
#else
            return unsigned(((occ & mask) * magic) >> shift);
#endif
        }
    };

    static std::array<Magic, 64> MAGICTABLE_BISHOP;
    static std::array<Magic, 64> MAGICTABLE_ROOK;

    using AttackTable = std::array<BitBoard, 64>;

    static const std::array<AttackTable, 2> ATTACK_TABLE_PAWN = []() {
        std::array<AttackTable, 2> table;

        // White pawn attacks
        for (int r = Rank::RANK_1; r <= Rank::RANK_8; r++)
        {
            for (int f = File::FILE_A; f <= File::FILE_H; f++)
            {
                const Square   sq = rf2sq(r, f);
                const BitBoard b  = BB(sq);
                const BitBoard attacks =
                  shift<Direction::NORTH_EAST>(b) | shift<Direction::NORTH_WEST>(b);
                table[Color::WHITE][sq] = attacks;
            }
        }

        // Black pawn attacks
        for (int r = Rank::RANK_8; r >= Rank::RANK_1; r--)
        {
            for (int f = File::FILE_A; f <= File::FILE_H; f++)
            {
                const Square   sq = rf2sq(r, f);
                const BitBoard b  = BB(sq);
                const BitBoard attacks =
                  shift<Direction::SOUTH_EAST>(b) | shift<Direction::SOUTH_WEST>(b);
                table[Color::BLACK][sq] = attacks;
            }
        }

        return table;
    }();

    static const AttackTable ATTACK_TABLE_KNIGHT = []() {
        AttackTable table;

        for (int sq = Square::A1; sq <= Square::H8; sq++)
        {
            const BitBoard b = BB(sq);

            BitBoard attacks = 0ULL;
            attacks |= (b & ~FILE_H_BB) << 17;
            attacks |= (b & ~(FILE_G_BB | FILE_H_BB)) << 10;
            attacks |= (b & ~(FILE_G_BB | FILE_H_BB)) >> 6;
            attacks |= (b & ~FILE_H_BB) >> 15;
            attacks |= (b & ~FILE_A_BB) << 15;
            attacks |= (b & ~(FILE_A_BB | FILE_B_BB)) << 6;
            attacks |= (b & ~(FILE_A_BB | FILE_B_BB)) >> 10;
            attacks |= (b & ~FILE_A_BB) >> 17;

            table[sq] = attacks;
        }

        return table;
    }();

    static const AttackTable ATTACK_TABLE_KING = []() {
        AttackTable table;

        for (int sq = Square::A1; sq <= Square::H8; sq++)
        {
            const BitBoard b = BB(sq);

            BitBoard attacks = 0ULL;
            attacks |= shift<Direction::NORTH>(b);
            attacks |= shift<Direction::SOUTH>(b);
            attacks |= shift<Direction::EAST>(b);
            attacks |= shift<Direction::WEST>(b);
            attacks |= shift<Direction::NORTH_EAST>(b);
            attacks |= shift<Direction::SOUTH_EAST>(b);
            attacks |= shift<Direction::SOUTH_WEST>(b);
            attacks |= shift<Direction::NORTH_WEST>(b);

            table[sq] = attacks;
        }

        return table;
    }();

    static std::array<AttackTable, 512>  ATTACK_TABLE_BISHOP;
    static std::array<AttackTable, 4096> ATTACK_TABLE_ROOK;

    static BitBoard bishopAttacks(const Square sq, const BitBoard blockers) {
        int r, f;

        const u8 tr = sq2rank(sq);
        const u8 tf = sq2file(sq);

        BitBoard attack_mask = 0ULL;

        for (r = tr + 1, f = tf + 1; r <= Rank::RANK_8 && f <= File::FILE_H; r++, f++)
        {
            const BitBoard sqb = BB(rf2sq(r, f));
            attack_mask |= sqb;
            if (blockers & sqb)
            {
                break;
            }
        }
        for (r = tr + 1, f = tf - 1; r <= Rank::RANK_8 && f >= File::FILE_A; r++, f--)
        {
            const BitBoard sqb = BB(rf2sq(r, f));
            attack_mask |= sqb;
            if (blockers & sqb)
            {
                break;
            }
        }
        for (r = tr - 1, f = tf + 1; r >= Rank::RANK_1 && f <= File::FILE_H; r--, f++)
        {
            const BitBoard sqb = BB(rf2sq(r, f));
            attack_mask |= sqb;
            if (blockers & sqb)
            {
                break;
            }
        }
        for (r = tr - 1, f = tf - 1; r >= Rank::RANK_1 && f >= File::FILE_A; r--, f--)
        {
            const BitBoard sqb = BB(rf2sq(r, f));
            attack_mask |= sqb;
            if (blockers & sqb)
            {
                break;
            }
        }

        return attack_mask;
    }

    static BitBoard rookAttacks(const Square sq, const BitBoard blockers) {
        int r, f;

        const u8 tr = sq2rank(sq);
        const u8 tf = sq2file(sq);

        BitBoard attack_mask = 0ULL;

        for (r = tr + 1; r <= Rank::RANK_8; r++)
        {
            const BitBoard sqb = BB(rf2sq(r, tf));
            attack_mask |= sqb;
            if (blockers & sqb)
            {
                break;
            }
        }
        for (r = tr - 1; r >= Rank::RANK_1; r--)
        {
            const BitBoard sqb = BB(rf2sq(r, tf));
            attack_mask |= sqb;
            if (blockers & sqb)
            {
                break;
            }
        }
        for (f = tf + 1; f <= File::FILE_H; f++)
        {
            const BitBoard sqb = BB(rf2sq(tr, f));
            attack_mask |= sqb;
            if (blockers & sqb)
            {
                break;
            }
        }
        for (f = tf - 1; f >= File::FILE_A; f--)
        {
            const BitBoard sqb = BB(rf2sq(tr, f));
            attack_mask |= sqb;
            if (blockers & sqb)
            {
                break;
            }
        }

        return attack_mask;
    }

    template<PieceType PT>
    static void initSliderAttackTable(std::array<Magic, 64>& magic_table,
                                      std::span<AttackTable> attack_table) {
        static constexpr size_t MAGIC_MAX_TRIES = 500000;

        const auto occupancy = [](const i32 index, const i32 bits, BitBoard mask) -> BitBoard {
            BitBoard result = 0ULL;

            for (i32 i = 0; i < bits; i++)
            {
                const i32 j = utils::bitScanForward(&mask);
                if (index & (1 << i))
                {
                    result |= BB(j);
                }
            }

            return result;
        };

        for (int sq = Square::A1; sq <= Square::H8; sq++)
        {
            const Square square = static_cast<Square>(sq);

            const BitBoard edges = ((RANK_1_BB | RANK_8_BB) & ~RANK_BB(sq2rank(sq)))
                                 | ((FILE_A_BB | FILE_H_BB) & ~FILE_BB(sq2file(sq)));

            Magic& m = magic_table[sq];

            m.mask          = (PT == PieceType::BISHOP) ? (bishopAttacks(square, 0ULL) & ~edges)
                            : (PT == PieceType::ROOK)   ? (rookAttacks(square, 0ULL) & ~edges)
                                                        : 0ULL;
            const auto bits = utils::bitCount1s(m.mask);
#if defined(IS_32_BIT)
            m.shift = 32 - bits;
#else
            m.shift = 64 - bits;
#endif

            BitBoard occupancies[4096];
            BitBoard attacks[4096];

            for (size_t i = 0; i < (1 << bits); i++)
            {
                occupancies[i] = occupancy(i, bits, m.mask);
                attacks[i]     = (PT == PieceType::BISHOP) ? bishopAttacks(square, occupancies[i])
                               : (PT == PieceType::ROOK)   ? rookAttacks(square, occupancies[i])
                                                           : 0ULL;
            }

            for (size_t tries = 0; tries < MAGIC_MAX_TRIES; ++tries)
            {
                for (m.magic = 0ULL; utils::bitCount1s((m.magic * m.magic) >> 56) < 6;)
                {
                    m.magic = utils::prng() & utils::prng() & utils::prng();
                }

                BitBoard used[4096] = {};
                bool     fail       = false;

                for (size_t i = 0; !fail && i < (1 << bits); i++)
                {
                    const auto index = m.index(occupancies[i]);
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
                    break;
                }
            }

            for (size_t i = 0; i < (1 << bits); i++)
            {
                const auto index        = m.index(occupancies[i]);
                attack_table[index][sq] = attacks[i];
            }
        }
    }

    template<Color US, MovegenType T>
    static void pseudolegalMovesPawn(containers::ArrayList<Move>* moves, const Position& pos) {
        assert(utils::bitCount1s(pos.checkers()) < 2);

        constexpr Color    them           = colorFlip(US);
        constexpr BitBoard promo_dest     = (US == Color::WHITE) ? RANK_8_BB : RANK_1_BB;
        constexpr BitBoard not_promo_dest = ~promo_dest;
        constexpr BitBoard ep_target_rank = (US == Color::WHITE) ? RANK_6_BB : RANK_3_BB;

        const BitBoard pawns     = pos.pieces(US, PieceType::PAWN);
        const BitBoard king_them = pos.pieces(them, PieceType::KING);
        const BitBoard enemies   = pos.pieces(them) ^ king_them;
        const BitBoard empty     = pos.empty();

        const Square   ep_target = pos.epTarget();
        const BitBoard ep_target_bb =
          (ep_target != Square::NO_SQ) ? (BB(ep_target) & ep_target_rank) : 0ULL;

        BitBoard pawns_fwd, sgl_push, dbl_push, fwd_l, fwd_r;

        if constexpr (US == Color::WHITE)
        {
            pawns_fwd = shift<Direction::NORTH>(pawns);
            sgl_push  = pawns_fwd & empty & not_promo_dest;
            dbl_push  = shift<Direction::NORTH>(sgl_push) & RANK_4_BB & empty;
            fwd_l     = shift<Direction::NORTH_WEST>(pawns);
            fwd_r     = shift<Direction::NORTH_EAST>(pawns);
        }
        else
        {
            pawns_fwd = shift<Direction::SOUTH>(pawns);
            sgl_push  = pawns_fwd & empty & not_promo_dest;
            dbl_push  = shift<Direction::SOUTH>(sgl_push) & RANK_5_BB & empty;
            fwd_l     = shift<Direction::SOUTH_EAST>(pawns);
            fwd_r     = shift<Direction::SOUTH_WEST>(pawns);
        }

        if constexpr (T == MovegenType::CHECK_EVASIONS)
        {
            BitBoard       checkers   = pos.checkers();
            const Square   checker_sq = static_cast<Square>(__builtin_ctzll(checkers));
            const BitBoard block_bb   = between(checker_sq, pos.kingSq());

            pawns_fwd &= block_bb;
            sgl_push &= block_bb;
            dbl_push &= block_bb;

            BitBoard fwd_mask_l = checkers;
            BitBoard fwd_mask_r = checkers;

            constexpr i8 ep_victim_dir = (US == Color::WHITE) ? 8 : -8;

            if (ep_target != Square::NO_SQ)
            {
                if ((checkers & BB(ep_target - ep_victim_dir)) || (block_bb & ep_target_bb))
                {
                    // En passant Capture checker
                    // Or, En passant Block check
                    fwd_mask_l |= (fwd_l & ep_target_bb);
                    fwd_mask_r |= (fwd_r & ep_target_bb);
                }
            }

            fwd_l &= fwd_mask_l;
            fwd_r &= fwd_mask_r;
        }

        const BitBoard enemies_not_on_promotion_dest = enemies & not_promo_dest;
        const BitBoard capture_l                     = fwd_l & enemies_not_on_promotion_dest;
        const BitBoard capture_r                     = fwd_r & enemies_not_on_promotion_dest;
        const BitBoard capture_ep_l                  = fwd_l & ep_target_bb;
        const BitBoard capture_ep_r                  = fwd_r & ep_target_bb;
        const BitBoard quite_promo                   = pawns_fwd & promo_dest & empty;
        const BitBoard enemies_on_promotion_dest     = enemies & promo_dest;
        const BitBoard capture_promo_l               = fwd_l & enemies_on_promotion_dest;
        const BitBoard capture_promo_r               = fwd_r & enemies_on_promotion_dest;

        BitBoard bb;
        int      dir;

        bb  = capture_promo_l;
        dir = (US == Color::WHITE) ? Direction::NORTH_WEST : Direction::SOUTH_EAST;
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
        dir = (US == Color::WHITE) ? Direction::NORTH_EAST : Direction::SOUTH_WEST;
        while (bb)
        {
            const Square to   = static_cast<Square>(utils::bitScanForward(&bb));
            const Square from = static_cast<Square>(to - dir);
            moves->emplace_back(from, to, MoveFlag::MOVE_CAPTURE_PROMOTION_QUEEN);
            moves->emplace_back(from, to, MoveFlag::MOVE_CAPTURE_PROMOTION_ROOK);
            moves->emplace_back(from, to, MoveFlag::MOVE_CAPTURE_PROMOTION_BISHOP);
            moves->emplace_back(from, to, MoveFlag::MOVE_CAPTURE_PROMOTION_KNIGHT);
        }

        dir = (US == Color::WHITE) ? Direction::NORTH_WEST : Direction::SOUTH_EAST;

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

        dir = (US == Color::WHITE) ? Direction::NORTH_EAST : Direction::SOUTH_WEST;

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

        if constexpr (T != MovegenType::CAPTURES)
        {
            dir = (US == Color::WHITE) ? Direction::NORTH : Direction::SOUTH;

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
            dir = (US == Color::WHITE) ? Direction::NORTH_2X : Direction::SOUTH_2X;
            while (bb)
            {
                const Square to   = static_cast<Square>(utils::bitScanForward(&bb));
                const Square from = static_cast<Square>(to - dir);
                moves->emplace_back(from, to, MoveFlag::MOVE_QUIET_PAWN_DBL_PUSH);
            }
        }
    }

    template<PieceType PT, Color US, MovegenType T>
    static void pseudolegalMovesPiece(containers::ArrayList<Move>* moves, const Position& pos) {
        constexpr Color them      = colorFlip(US);
        const BitBoard  king_them = pos.pieces(them, PieceType::KING);
        const BitBoard  enemies   = pos.pieces(them) ^ king_them;
        const BitBoard  occupied  = pos.occupied();
        const BitBoard  empty     = ~occupied;

        BitBoard checkers = 0ULL;
        BitBoard block_bb = 0ULL;
        if constexpr (T == MovegenType::CHECK_EVASIONS)
        {
            checkers                = pos.checkers();
            const Square checker_sq = static_cast<Square>(__builtin_ctzll(checkers));
            block_bb                = between(checker_sq, pos.kingSq());
        }

        const BitBoard occ =
          ((PT == PieceType::KNIGHT) || (PT == PieceType::KING)) ? (enemies | empty) : occupied;

        BitBoard bb = pos.pieces(US, PT);
        while (bb)
        {
            const Square   from  = static_cast<Square>(utils::bitScanForward(&bb));
            const BitBoard attks = attacks<PT>(from, occ);

            BitBoard captures = attks & enemies;
            if constexpr (T == MovegenType::CHECK_EVASIONS)
            {
                captures &= checkers;
            }
            while (captures)
            {
                const Square to = static_cast<Square>(utils::bitScanForward(&captures));
                moves->emplace_back(from, to, MoveFlag::MOVE_CAPTURE);
            }

            if constexpr (T != MovegenType::CAPTURES)
            {
                BitBoard quites = attks & empty;
                if constexpr (T == MovegenType::CHECK_EVASIONS)
                {
                    quites &= block_bb;
                }
                while (quites)
                {
                    const Square to = static_cast<Square>(utils::bitScanForward(&quites));
                    moves->emplace_back(from, to, MoveFlag::MOVE_QUIET);
                }
            }
        }
    }

    template<Color US>
    static void pseudolegalMovesCastle(containers::ArrayList<Move>* moves, const Position& pos) {
        assert(!pos.isInCheck());

        static constexpr BitBoard MASK_WKCA_PATH = 0x60;
        static constexpr BitBoard MASK_WQCA_PATH = 0xE;
        static constexpr BitBoard MASK_BKCA_PATH = 0x6000000000000000;
        static constexpr BitBoard MASK_BQCA_PATH = 0xE00000000000000;

        constexpr CastleFlag rights_k = (US == Color::WHITE) ? CastleFlag::WKCA : CastleFlag::BKCA;
        constexpr CastleFlag rights_q = (US == Color::WHITE) ? CastleFlag::WQCA : CastleFlag::BQCA;
        constexpr auto       ca_flags = rights_k | rights_q;

        const auto pos_ca_rights = pos.caRights();

        if (!(pos_ca_rights & ca_flags))
        {
            return;
        }

        constexpr Color  them   = colorFlip(US);
        constexpr Rank   rank   = (US == Color::WHITE) ? Rank::RANK_1 : Rank::RANK_8;
        constexpr Square k_sq   = rf2sq(rank, File::FILE_E);
        constexpr Square k_sq_r = static_cast<Square>(k_sq + 1);
        constexpr Square k_sq_l = static_cast<Square>(k_sq - 1);

        const BitBoard occ = pos.occupied();

        const bool rights_k_ok = (pos_ca_rights & rights_k);
        const bool rights_q_ok = (pos_ca_rights & rights_q);

        const BitBoard rook_bb   = pos.pieces(US, PieceType::ROOK);
        const BitBoard king_bb   = pos.pieces(US, PieceType::KING);
        const bool     rook_k_ok = (rook_bb & BB(rf2sq(rank, File::FILE_H)));
        const bool     rook_q_ok = (rook_bb & BB(rf2sq(rank, File::FILE_A)));
        const bool     king_ok   = (king_bb & BB(rf2sq(rank, File::FILE_E)));

        constexpr BitBoard mask_path_k  = (US == Color::WHITE) ? MASK_WKCA_PATH : MASK_BKCA_PATH;
        const bool         path_k_empty = ((occ & mask_path_k) == 0ULL);

        constexpr BitBoard mask_path_q  = (US == Color::WHITE) ? MASK_WQCA_PATH : MASK_BQCA_PATH;
        const bool         path_q_empty = ((occ & mask_path_q) == 0ULL);

        const bool safe_k_ok = (squareAttackers(pos, k_sq_r, them) == 0ULL);
        const bool safe_q_ok = (squareAttackers(pos, k_sq_l, them) == 0ULL);

        if (rights_k_ok && king_ok && rook_k_ok && path_k_empty && safe_k_ok)
        {
            constexpr Square from = rf2sq(rank, File::FILE_E);
            constexpr Square to   = rf2sq(rank, File::FILE_G);
            moves->emplace_back(from, to, MoveFlag::MOVE_CASTLE_KING_SIDE);
        }

        if (rights_q_ok && king_ok && rook_q_ok && path_q_empty && safe_q_ok)
        {
            constexpr Square from = rf2sq(rank, File::FILE_E);
            constexpr Square to   = rf2sq(rank, File::FILE_C);
            moves->emplace_back(from, to, MoveFlag::MOVE_CASTLE_QUEEN_SIDE);
        }
    }

    void movegen_initialize() {
        initSliderAttackTable<PieceType::BISHOP>(MAGICTABLE_BISHOP, std::span(ATTACK_TABLE_BISHOP));
        initSliderAttackTable<PieceType::ROOK>(MAGICTABLE_ROOK, std::span(ATTACK_TABLE_ROOK));
    }

    template<PieceType PT>
    BitBoard attacks(const Square sq, const BitBoard occupancy, const Color c) {
        switch (PT)
        {
            case PieceType::PAWN :
                return ATTACK_TABLE_PAWN[c][sq] & occupancy;

            case PieceType::KNIGHT :
                return ATTACK_TABLE_KNIGHT[sq] & occupancy;

            case PieceType::BISHOP : {
                const auto index = MAGICTABLE_BISHOP[sq].index(occupancy);
                return ATTACK_TABLE_BISHOP[index][sq];
            }

            case PieceType::ROOK : {
                const auto index = MAGICTABLE_ROOK[sq].index(occupancy);
                return ATTACK_TABLE_ROOK[index][sq];
            }

            case PieceType::QUEEN : {
                const auto b_index = MAGICTABLE_BISHOP[sq].index(occupancy);
                const auto r_index = MAGICTABLE_ROOK[sq].index(occupancy);
                return (ATTACK_TABLE_BISHOP[b_index][sq] | ATTACK_TABLE_ROOK[r_index][sq]);
            }

            case PieceType::KING :
                return ATTACK_TABLE_KING[sq] & occupancy;

            default :
                assert(false);
                break;
        }

        return 0ULL;
    }

    BitBoard squareAttackers(const Position& pos, const Square sq, const Color attacked_by) {
        const BitBoard occ  = pos.occupied();
        const BitBoard op_P = pos.pieces(attacked_by, PieceType::PAWN);
        const BitBoard op_N = pos.pieces(attacked_by, PieceType::KNIGHT);
        BitBoard       op_RQ, op_BQ;
        op_RQ = op_BQ = pos.pieces(attacked_by, PieceType::QUEEN);
        op_RQ |= pos.pieces(attacked_by, PieceType::ROOK);
        op_BQ |= pos.pieces(attacked_by, PieceType::BISHOP);
        const BitBoard op_K = pos.pieces(attacked_by, PieceType::KING);
        // clang-format off
        return attacks<PieceType::PAWN>(sq, op_P, colorFlip(attacked_by))
             | attacks<PieceType::KNIGHT>(sq, op_N)
             | (attacks<PieceType::BISHOP>(sq, occ) & op_BQ)
             | (attacks<PieceType::ROOK>(sq, occ) & op_RQ)
             | attacks<PieceType::KING>(sq, op_K);
        // clang-format on
    }

    template<Color US, MovegenType T>
    static void pseudolegalMovesColor(containers::ArrayList<Move>* moves, const Position& pos) {
        pseudolegalMovesPiece<PieceType::KING, US, T>(moves, pos);

        if constexpr (T != MovegenType::CAPTURES)
        {
            const BitBoard checkers = pos.checkers();
            if (checkers || (T == MovegenType::CHECK_EVASIONS))
            {
                if (checkers & (checkers - 1)) [[unlikely]]
                {
                    // Multiple checkers
                    // Only King moves allowed
                    return;
                }

                pseudolegalMovesPawn<US, MovegenType::CHECK_EVASIONS>(moves, pos);
                pseudolegalMovesPiece<PieceType::QUEEN, US, MovegenType::CHECK_EVASIONS>(moves,
                                                                                         pos);
                pseudolegalMovesPiece<PieceType::ROOK, US, MovegenType::CHECK_EVASIONS>(moves, pos);
                pseudolegalMovesPiece<PieceType::BISHOP, US, MovegenType::CHECK_EVASIONS>(moves,
                                                                                          pos);
                pseudolegalMovesPiece<PieceType::KNIGHT, US, MovegenType::CHECK_EVASIONS>(moves,
                                                                                          pos);

                return;
            }
        }

        pseudolegalMovesPawn<US, T>(moves, pos);
        pseudolegalMovesPiece<PieceType::QUEEN, US, T>(moves, pos);
        pseudolegalMovesPiece<PieceType::ROOK, US, T>(moves, pos);
        pseudolegalMovesPiece<PieceType::BISHOP, US, T>(moves, pos);
        pseudolegalMovesPiece<PieceType::KNIGHT, US, T>(moves, pos);
        if constexpr (T == MovegenType::ALL)
        {
            pseudolegalMovesCastle<US>(moves, pos);
        }
    }

    template<MovegenType T>
    void pseudolegalMoves(containers::ArrayList<Move>* moves, const Position& pos) {
        if (pos.stm() == Color::WHITE)
        {
            pseudolegalMovesColor<Color::WHITE, T>(moves, pos);
        }
        else
        {
            pseudolegalMovesColor<Color::BLACK, T>(moves, pos);
        }
    }

    template void pseudolegalMoves<MovegenType::ALL>(containers::ArrayList<Move>* moves,
                                                     const Position&              pos);
    template void pseudolegalMoves<MovegenType::CAPTURES>(containers::ArrayList<Move>* moves,
                                                          const Position&              pos);
    template void pseudolegalMoves<MovegenType::CHECK_EVASIONS>(containers::ArrayList<Move>* moves,
                                                                const Position&              pos);
}
