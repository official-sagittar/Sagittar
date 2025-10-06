#include "movegen.h"
#include "core/utils.h"

namespace sagittar {

    namespace core {

        static constexpr int MAGIC_MAX_TRIES = 500000;

        static constexpr BitBoard BITBOARD_MASK_RANK_1      = 0x00000000000000FF;
        static constexpr BitBoard BITBOARD_MASK_NOT_RANK_1  = ~BITBOARD_MASK_RANK_1;
        static constexpr BitBoard BITBOARD_MASK_RANK_3      = 0x0000000000FF0000;
        static constexpr BitBoard BITBOARD_MASK_RANK_4      = 0x00000000FF000000;
        static constexpr BitBoard BITBOARD_MASK_RANK_5      = 0x000000FF00000000;
        static constexpr BitBoard BITBOARD_MASK_RANK_6      = 0x0000FF0000000000;
        static constexpr BitBoard BITBOARD_MASK_RANK_8      = 0xFF00000000000000;
        static constexpr BitBoard BITBOARD_MASK_NOT_RANK_8  = ~BITBOARD_MASK_RANK_8;
        static constexpr BitBoard BITBOARD_MASK_NOT_A_FILE  = 0xFEFEFEFEFEFEFEFE;
        static constexpr BitBoard BITBOARD_MASK_NOT_H_FILE  = 0x7F7F7F7F7F7F7F7F;
        static constexpr BitBoard BITBOARD_MASK_NOT_AB_FILE = 0xFCFCFCFCFCFCFCFC;
        static constexpr BitBoard BITBOARD_MASK_NOT_GH_FILE = 0x3F3F3F3F3F3F3F3F;
        static constexpr BitBoard BITBOARD_MASK_WKCA_PATH   = 0x60;
        static constexpr BitBoard BITBOARD_MASK_WQCA_PATH   = 0xE;
        static constexpr BitBoard BITBOARD_MASK_BKCA_PATH   = 0x6000000000000000;
        static constexpr BitBoard BITBOARD_MASK_BQCA_PATH   = 0xE00000000000000;

        static constexpr BitBoard BITBOARD_NORTH(const BitBoard b) { return b << 8; }
        static constexpr BitBoard BITBOARD_SOUTH(const BitBoard b) { return b >> 8; }
        static constexpr BitBoard BITBOARD_EAST(const BitBoard b) {
            return (b & BITBOARD_MASK_NOT_H_FILE) << 1;
        }
        static constexpr BitBoard BITBOARD_WEST(const BitBoard b) {
            return (b & BITBOARD_MASK_NOT_A_FILE) >> 1;
        }
        static constexpr BitBoard BITBOARD_NORTH_EAST(const BitBoard b) {
            return (b & BITBOARD_MASK_NOT_H_FILE) << 9;
        }
        static constexpr BitBoard BITBOARD_SOUTH_EAST(const BitBoard b) {
            return (b & BITBOARD_MASK_NOT_H_FILE) >> 7;
        }
        static constexpr BitBoard BITBOARD_SOUTH_WEST(const BitBoard b) {
            return (b & BITBOARD_MASK_NOT_A_FILE) >> 9;
        }
        static constexpr BitBoard BITBOARD_NORTH_WEST(const BitBoard b) {
            return (b & BITBOARD_MASK_NOT_A_FILE) << 7;
        }

        template<Color US>
        static constexpr int BITBOARD_FWD_DIR = (US == WHITE) ? 8 : -8;
        template<Color US>
        static constexpr int BITBOARD_FWD_DBL_DIR = (US == WHITE) ? 16 : -16;
        template<Color US>
        static constexpr int BITBOARD_CAPTURE_LEFT_DIR = (US == WHITE) ? 7 : -7;
        template<Color US>
        static constexpr int BITBOARD_CAPTURE_RIGHT_DIR = (US == WHITE) ? 9 : -9;

        struct Magic {
            BitBoard mask;
            uint64_t magic;
            uint8_t  shift;
        };

        static struct Magic MAGICTABLE_BISHOP[64];
        static struct Magic MAGICTABLE_ROOK[64];

        static BitBoard ATTACK_TABLE_PAWN[2][64];
        static BitBoard ATTACK_TABLE_KNIGHT[64];
        static BitBoard ATTACK_TABLE_BISHOP[64][512];
        static BitBoard ATTACK_TABLE_ROOK[64][4096];
        static BitBoard ATTACK_TABLE_KING[64];

        static BitBoard LINE_BB[64][64];
        static BitBoard RAY_BB[64][64];

        static void movegen_init_line_and_ray_bb() {
            for (int sq1 = 0; sq1 < 64; ++sq1)
            {
                int r1 = sq1 / 8, f1 = sq1 % 8;

                for (int sq2 = 0; sq2 < 64; ++sq2)
                {
                    int r2 = sq2 / 8, f2 = sq2 % 8;

                    BitBoard line_mask = 0ULL;
                    BitBoard ray_mask  = 0ULL;

                    // skip identical square case
                    if (sq1 == sq2)
                    {
                        LINE_BB[sq1][sq2] = 0ULL;
                        RAY_BB[sq1][sq2]  = 0ULL;
                        continue;
                    }

                    // check alignment
                    bool same_rank  = (r1 == r2);
                    bool same_file  = (f1 == f2);
                    bool same_diag  = (r1 - f1 == r2 - f2);
                    bool same_antid = (r1 + f1 == r2 + f2);

                    if (same_rank || same_file || same_diag || same_antid)
                    {
                        int dr = (r2 > r1) - (r2 < r1);  // -1, 0, +1
                        int df = (f2 > f1) - (f2 < f1);

                        int r = r1, f = f1;
                        ray_mask |= 1ULL << (r * 8 + f);  // start sq1

                        // walk along direction until we reach sq2
                        while (r != r2 || f != f2)
                        {
                            r += dr;
                            f += df;
                            int sq = r * 8 + f;
                            ray_mask |= 1ULL << sq;

                            if (r != r2 || f != f2)
                                line_mask |= 1ULL << sq;  // add intermediate squares
                        }
                    }

                    LINE_BB[sq1][sq2] = line_mask;
                    RAY_BB[sq1][sq2]  = ray_mask;
                }
            }
        }

        static BitBoard movegen_bishop_mask(const Square sq) {
            int r, f;

            const int tr = SQ_TO_RANK(sq);
            const int tf = SQ_TO_FILE(sq);

            BitBoard attack_mask = 0ULL;

            for (r = tr + 1, f = tf + 1; r <= 6 && f <= 6; r++, f++)
            {
                attack_mask |= (1ULL << RF_TO_SQ(r, f));
            }
            for (r = tr + 1, f = tf - 1; r <= 6 && f >= 1; r++, f--)
            {
                attack_mask |= (1ULL << RF_TO_SQ(r, f));
            }
            for (r = tr - 1, f = tf + 1; r >= 1 && f <= 6; r--, f++)
            {
                attack_mask |= (1ULL << RF_TO_SQ(r, f));
            }
            for (r = tr - 1, f = tf - 1; r >= 1 && f >= 1; r--, f--)
            {
                attack_mask |= (1ULL << RF_TO_SQ(r, f));
            }

            return attack_mask;
        }

        static BitBoard movegen_bishop_attacks(const Square sq, const BitBoard blockers) {
            int r, f;

            const int tr = SQ_TO_RANK(sq);
            const int tf = SQ_TO_FILE(sq);

            BitBoard attack_mask = 0ULL;
            BitBoard sqb;

            for (r = tr + 1, f = tf + 1; r <= 7 && f <= 7; r++, f++)
            {
                sqb = 1ULL << RF_TO_SQ(r, f);
                attack_mask |= sqb;
                if (blockers & sqb)
                {
                    break;
                }
            }
            for (r = tr + 1, f = tf - 1; r <= 7 && f >= 0; r++, f--)
            {
                sqb = 1ULL << RF_TO_SQ(r, f);
                attack_mask |= sqb;
                if (blockers & sqb)
                {
                    break;
                }
            }
            for (r = tr - 1, f = tf + 1; r >= 0 && f <= 7; r--, f++)
            {
                sqb = 1ULL << RF_TO_SQ(r, f);
                attack_mask |= sqb;
                if (blockers & sqb)
                {
                    break;
                }
            }
            for (r = tr - 1, f = tf - 1; r >= 0 && f >= 0; r--, f--)
            {
                sqb = 1ULL << RF_TO_SQ(r, f);
                attack_mask |= sqb;
                if (blockers & sqb)
                {
                    break;
                }
            }

            return attack_mask;
        }

        static BitBoard movegen_rook_mask(const Square sq) {
            int r, f;

            const int tr = SQ_TO_RANK(sq);
            const int tf = SQ_TO_FILE(sq);

            BitBoard attack_mask = 0ULL;

            for (r = tr + 1; r <= 6; r++)
            {
                attack_mask |= (1ULL << RF_TO_SQ(r, tf));
            }
            for (r = tr - 1; r >= 1; r--)
            {
                attack_mask |= (1ULL << RF_TO_SQ(r, tf));
            }
            for (f = tf + 1; f <= 6; f++)
            {
                attack_mask |= (1ULL << RF_TO_SQ(tr, f));
            }
            for (f = tf - 1; f >= 1; f--)
            {
                attack_mask |= (1ULL << RF_TO_SQ(tr, f));
            }

            return attack_mask;
        }

        static BitBoard movegen_rook_attacks(const Square sq, const BitBoard blockers) {
            int r, f;

            const int tr = SQ_TO_RANK(sq);
            const int tf = SQ_TO_FILE(sq);

            BitBoard attack_mask = 0ULL;
            BitBoard sqb;

            for (r = tr + 1; r <= 7; r++)
            {
                sqb = 1ULL << RF_TO_SQ(r, tf);
                attack_mask |= sqb;
                if (blockers & sqb)
                {
                    break;
                }
            }
            for (r = tr - 1; r >= 0; r--)
            {
                sqb = 1ULL << RF_TO_SQ(r, tf);
                attack_mask |= sqb;
                if (blockers & sqb)
                {
                    break;
                }
            }
            for (f = tf + 1; f <= 7; f++)
            {
                sqb = 1ULL << RF_TO_SQ(tr, f);
                attack_mask |= sqb;
                if (blockers & sqb)
                {
                    break;
                }
            }
            for (f = tf - 1; f >= 0; f--)
            {
                sqb = 1ULL << RF_TO_SQ(tr, f);
                attack_mask |= sqb;
                if (blockers & sqb)
                {
                    break;
                }
            }

            return attack_mask;
        }

        static BitBoard
        movegen_create_blocker_variant(const int index, const int bits, BitBoard mask) {
            int      i, j;
            BitBoard result = 0ULL;
            for (i = 0; i < bits; i++)
            {
                j = bitscan_forward(&mask);
                if (index & (1 << i))
                {
                    result |= (1ULL << j);
                }
            }
            return result;
        }

        static uint32_t
        movegen_transform_magic_to_index(const BitBoard b, const BitBoard magic, const int bits) {
            return (uint32_t) ((b * magic) >> (64 - bits));
        }

        static uint64_t movegen_create_random_magic() { return prng() & prng() & prng(); }

        static uint64_t movegen_find_magic(const Square sq, const int bits, const bool bishop) {
            const BitBoard mask = bishop ? movegen_bishop_mask(sq) : movegen_rook_mask(sq);
            const int      n    = POPCNT(mask);

            BitBoard attacks[4096];
            BitBoard variants[4096];

            for (int i = 0; i < (1 << n); i++)
            {
                variants[i] = movegen_create_blocker_variant(i, n, mask);
                attacks[i]  = bishop ? movegen_bishop_attacks(sq, variants[i])
                                     : movegen_rook_attacks(sq, variants[i]);
            }

            for (int tries = 0; tries < MAGIC_MAX_TRIES; tries++)
            {
                const uint64_t magic = movegen_create_random_magic();
                if (POPCNT((mask * magic) & 0xFF00000000000000ULL) >= 6)
                {
                    continue;
                }

                BitBoard used[4096] = {};
                bool     fail       = false;

                for (int i = 0; !fail && i < (1 << n); i++)
                {
                    const uint32_t index =
                      movegen_transform_magic_to_index(variants[i], magic, bits);
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

        static void movegen_init_magic_table_bishop() {
            for (int sq = A1; sq <= H8; sq++)
            {
                MAGICTABLE_BISHOP[sq].mask  = movegen_bishop_mask(static_cast<Square>(sq));
                MAGICTABLE_BISHOP[sq].shift = POPCNT(MAGICTABLE_BISHOP[sq].mask);
                MAGICTABLE_BISHOP[sq].magic =
                  movegen_find_magic(static_cast<Square>(sq), MAGICTABLE_BISHOP[sq].shift, true);
                assert(MAGICTABLE_BISHOP[sq].magic != 0ULL);
            }
        }

        static void movegen_init_magic_table_rook() {
            for (int sq = A1; sq <= H8; sq++)
            {
                MAGICTABLE_ROOK[sq].mask  = movegen_rook_mask(static_cast<Square>(sq));
                MAGICTABLE_ROOK[sq].shift = POPCNT(MAGICTABLE_ROOK[sq].mask);
                MAGICTABLE_ROOK[sq].magic =
                  movegen_find_magic(static_cast<Square>(sq), MAGICTABLE_ROOK[sq].shift, false);
                assert(MAGICTABLE_ROOK[sq].magic != 0ULL);
            }
        }

        static void movegen_init_attack_table_pawn() {
            Square   sq;
            BitBoard b;
            BitBoard attacks;

            // White pawn attacks
            for (int r = RANK_1; r <= RANK_8; r++)
            {
                for (int f = FILE_A; f <= FILE_H; f++)
                {
                    sq                           = RF_TO_SQ(r, f);
                    b                            = 1ULL << sq;
                    attacks                      = BITBOARD_NORTH_EAST(b) | BITBOARD_NORTH_WEST(b);
                    ATTACK_TABLE_PAWN[WHITE][sq] = attacks;
                }
            }

            // Black pawn attacks
            for (int r = RANK_8; r >= RANK_1; r--)
            {
                for (int f = FILE_A; f <= FILE_H; f++)
                {
                    sq                           = RF_TO_SQ(r, f);
                    b                            = 1ULL << sq;
                    attacks                      = BITBOARD_SOUTH_EAST(b) | BITBOARD_SOUTH_WEST(b);
                    ATTACK_TABLE_PAWN[BLACK][sq] = attacks;
                }
            }
        }

        static void movegen_init_attack_table_knight() {
            BitBoard attacks = 0ULL;
            BitBoard b;

            for (int sq = A1; sq <= H8; sq++)
            {
                b = 1ULL << sq;

                attacks = 0ULL;
                attacks |= (b & BITBOARD_MASK_NOT_H_FILE) << 17;
                attacks |= (b & BITBOARD_MASK_NOT_GH_FILE) << 10;
                attacks |= (b & BITBOARD_MASK_NOT_GH_FILE) >> 6;
                attacks |= (b & BITBOARD_MASK_NOT_H_FILE) >> 15;
                attacks |= (b & BITBOARD_MASK_NOT_A_FILE) << 15;
                attacks |= (b & BITBOARD_MASK_NOT_AB_FILE) << 6;
                attacks |= (b & BITBOARD_MASK_NOT_AB_FILE) >> 10;
                attacks |= (b & BITBOARD_MASK_NOT_A_FILE) >> 17;

                ATTACK_TABLE_KNIGHT[sq] = attacks;
            }
        }

        static void movegen_init_attack_table_bishop() {
            BitBoard mask, b;
            uint8_t  n;
            uint32_t magic_index;

            for (uint8_t sq = A1; sq <= H8; sq++)
            {
                mask = MAGICTABLE_BISHOP[sq].mask;
                n    = MAGICTABLE_BISHOP[sq].shift;

                for (int32_t i = 0; i < (1 << n); i++)
                {
                    b = movegen_create_blocker_variant(i, n, mask);
                    magic_index =
                      movegen_transform_magic_to_index(b, MAGICTABLE_BISHOP[sq].magic, n);
                    ATTACK_TABLE_BISHOP[sq][magic_index] =
                      movegen_bishop_attacks(static_cast<Square>(sq), b);
                }
            }
        }

        static void movegen_init_attack_table_rook() {
            BitBoard mask, b;
            uint8_t  n;
            uint32_t magic_index;

            for (uint8_t sq = 0; sq < 64; sq++)
            {
                mask = MAGICTABLE_ROOK[sq].mask;
                n    = MAGICTABLE_ROOK[sq].shift;

                for (int32_t i = 0; i < (1 << n); i++)
                {
                    b           = movegen_create_blocker_variant(i, n, mask);
                    magic_index = movegen_transform_magic_to_index(b, MAGICTABLE_ROOK[sq].magic, n);
                    ATTACK_TABLE_ROOK[sq][magic_index] =
                      movegen_rook_attacks(static_cast<Square>(sq), b);
                }
            }
        }

        static void movegen_init_attack_table_king() {
            BitBoard attacks = 0ULL;
            BitBoard b;

            for (int sq = 0; sq < 64; sq++)
            {
                b = 1ULL << sq;

                attacks = 0ULL;
                attacks |= BITBOARD_NORTH(b);
                attacks |= BITBOARD_SOUTH(b);
                attacks |= BITBOARD_EAST(b);
                attacks |= BITBOARD_WEST(b);
                attacks |= BITBOARD_NORTH_EAST(b);
                attacks |= BITBOARD_SOUTH_EAST(b);
                attacks |= BITBOARD_SOUTH_WEST(b);
                attacks |= BITBOARD_NORTH_WEST(b);

                ATTACK_TABLE_KING[sq] = attacks;
            }
        }

        BitBoard movegen_get_bishop_attacks(const Square sq, BitBoard occupancy) {
            occupancy            = occupancy & MAGICTABLE_BISHOP[sq].mask;
            const uint32_t index = movegen_transform_magic_to_index(
              occupancy, MAGICTABLE_BISHOP[sq].magic, MAGICTABLE_BISHOP[sq].shift);
            return ATTACK_TABLE_BISHOP[sq][index];
        }

        BitBoard movegen_get_rook_attacks(const Square sq, BitBoard occupancy) {
            occupancy            = occupancy & MAGICTABLE_ROOK[sq].mask;
            const uint32_t index = movegen_transform_magic_to_index(
              occupancy, MAGICTABLE_ROOK[sq].magic, MAGICTABLE_ROOK[sq].shift);
            return ATTACK_TABLE_ROOK[sq][index];
        }

        template<Color US, MovegenType T>
        static void movegen_generate_pseudolegal_moves_pawn(const Position& pos,
                                                            MoveList* const move_list) {
            constexpr Color    them = COLOR_FLIP(US);
            constexpr BitBoard promo_dest =
              (US == WHITE) ? BITBOARD_MASK_RANK_8 : BITBOARD_MASK_RANK_1;
            constexpr BitBoard not_promo_dest =
              (US == WHITE) ? BITBOARD_MASK_NOT_RANK_8 : BITBOARD_MASK_NOT_RANK_1;
            constexpr BitBoard ep_target_rank =
              (US == WHITE) ? BITBOARD_MASK_RANK_6 : BITBOARD_MASK_RANK_3;

            const BitBoard pawns = pos.board.bb_pieces[PAWN] & pos.board.bb_colors[US];
            const BitBoard enemies =
              pos.board.bb_colors[them] ^ (pos.board.bb_pieces[KING] & pos.board.bb_colors[them]);
            const BitBoard empty = ~(pos.board.bb_colors[WHITE] | pos.board.bb_colors[BLACK]);

            BitBoard pawns_fwd, sgl_push, dbl_push, fwd_l, fwd_r;
            if constexpr (US == WHITE)
            {
                pawns_fwd = BITBOARD_NORTH(pawns);
                sgl_push  = pawns_fwd & empty & not_promo_dest;
                dbl_push  = BITBOARD_NORTH(sgl_push) & BITBOARD_MASK_RANK_4 & empty;
                fwd_l     = BITBOARD_NORTH_WEST(pawns);
                fwd_r     = BITBOARD_NORTH_EAST(pawns);
            }
            else
            {
                pawns_fwd = BITBOARD_SOUTH(pawns);
                sgl_push  = pawns_fwd & empty & not_promo_dest;
                dbl_push  = BITBOARD_SOUTH(sgl_push) & BITBOARD_MASK_RANK_5 & empty;
                fwd_l     = BITBOARD_SOUTH_EAST(pawns);
                fwd_r     = BITBOARD_SOUTH_WEST(pawns);
            }

            const BitBoard enemies_not_on_promotion_dest = enemies & not_promo_dest;
            const BitBoard capture_l                     = fwd_l & enemies_not_on_promotion_dest;
            const BitBoard capture_r                     = fwd_r & enemies_not_on_promotion_dest;
            const BitBoard ep_target_bb                  = BB(pos.ep_target) & ep_target_rank;
            const BitBoard capture_ep_l                  = fwd_l & ep_target_bb;
            const BitBoard capture_ep_r                  = fwd_r & ep_target_bb;
            const BitBoard quite_promo                   = pawns_fwd & promo_dest & empty;
            const BitBoard enemies_on_promotion_dest     = enemies & promo_dest;
            const BitBoard capture_promo_l               = fwd_l & enemies_on_promotion_dest;
            const BitBoard capture_promo_r               = fwd_r & enemies_on_promotion_dest;

            BitBoard bb;
            int      dir;

            if constexpr (T == MovegenType::MOVEGEN_ALL)
            {
                bb  = sgl_push;
                dir = BITBOARD_FWD_DIR<US>;
                while (bb)
                {
                    const Square to   = static_cast<Square>(bitscan_forward(&bb));
                    const Square from = static_cast<Square>(to - dir);
                    move_list->add_move(MOVE_CREATE(from, to, MOVE_QUIET));
                }

                bb  = dbl_push;
                dir = BITBOARD_FWD_DBL_DIR<US>;
                while (bb)
                {
                    const Square to   = static_cast<Square>(bitscan_forward(&bb));
                    const Square from = static_cast<Square>(to - dir);
                    move_list->add_move(MOVE_CREATE(from, to, MOVE_QUIET_PAWN_DBL_PUSH));
                }
            }

            bb  = capture_l;
            dir = BITBOARD_CAPTURE_LEFT_DIR<US>;
            while (bb)
            {
                const Square to   = static_cast<Square>(bitscan_forward(&bb));
                const Square from = static_cast<Square>(to - dir);
                move_list->add_move(MOVE_CREATE(from, to, MOVE_CAPTURE));
            }

            bb  = capture_r;
            dir = BITBOARD_CAPTURE_RIGHT_DIR<US>;
            while (bb)
            {
                const Square to   = static_cast<Square>(bitscan_forward(&bb));
                const Square from = static_cast<Square>(to - dir);
                move_list->add_move(MOVE_CREATE(from, to, MOVE_CAPTURE));
            }

            bb  = capture_ep_l;
            dir = BITBOARD_CAPTURE_LEFT_DIR<US>;
            while (bb)
            {
                const Square to   = static_cast<Square>(bitscan_forward(&bb));
                const Square from = static_cast<Square>(to - dir);
                move_list->add_move(MOVE_CREATE(from, to, MOVE_CAPTURE_EP));
            }

            bb  = capture_ep_r;
            dir = BITBOARD_CAPTURE_RIGHT_DIR<US>;
            while (bb)
            {
                const Square to   = static_cast<Square>(bitscan_forward(&bb));
                const Square from = static_cast<Square>(to - dir);
                move_list->add_move(MOVE_CREATE(from, to, MOVE_CAPTURE_EP));
            }

            if constexpr (T == MovegenType::MOVEGEN_ALL)
            {
                bb  = quite_promo;
                dir = BITBOARD_FWD_DIR<US>;
                while (bb)
                {
                    const Square to   = static_cast<Square>(bitscan_forward(&bb));
                    const Square from = static_cast<Square>(to - dir);
                    move_list->add_move(MOVE_CREATE(from, to, MOVE_PROMOTION_QUEEN));
                    move_list->add_move(MOVE_CREATE(from, to, MOVE_PROMOTION_ROOK));
                    move_list->add_move(MOVE_CREATE(from, to, MOVE_PROMOTION_KNIGHT));
                    move_list->add_move(MOVE_CREATE(from, to, MOVE_PROMOTION_BISHOP));
                }
            }

            bb  = capture_promo_l;
            dir = BITBOARD_CAPTURE_LEFT_DIR<US>;
            while (bb)
            {
                const Square to   = static_cast<Square>(bitscan_forward(&bb));
                const Square from = static_cast<Square>(to - dir);
                move_list->add_move(MOVE_CREATE(from, to, MOVE_CAPTURE_PROMOTION_QUEEN));
                move_list->add_move(MOVE_CREATE(from, to, MOVE_CAPTURE_PROMOTION_ROOK));
                move_list->add_move(MOVE_CREATE(from, to, MOVE_CAPTURE_PROMOTION_KNIGHT));
                move_list->add_move(MOVE_CREATE(from, to, MOVE_CAPTURE_PROMOTION_BISHOP));
            }

            bb  = capture_promo_r;
            dir = BITBOARD_CAPTURE_RIGHT_DIR<US>;
            while (bb)
            {
                const Square to   = static_cast<Square>(bitscan_forward(&bb));
                const Square from = static_cast<Square>(to - dir);
                move_list->add_move(MOVE_CREATE(from, to, MOVE_CAPTURE_PROMOTION_QUEEN));
                move_list->add_move(MOVE_CREATE(from, to, MOVE_CAPTURE_PROMOTION_ROOK));
                move_list->add_move(MOVE_CREATE(from, to, MOVE_CAPTURE_PROMOTION_KNIGHT));
                move_list->add_move(MOVE_CREATE(from, to, MOVE_CAPTURE_PROMOTION_BISHOP));
            }
        }

        template<PieceType PT, Color US, MovegenType T>
        static void movegen_generate_pseudolegal_moves_piece(const Position& pos,
                                                             MoveList* const move_list) {
            constexpr Color them = COLOR_FLIP(US);
            const BitBoard  enemies =
              pos.board.bb_colors[them] ^ (pos.board.bb_pieces[KING] & pos.board.bb_colors[them]);
            const BitBoard empty = ~(pos.board.bb_colors[WHITE] | pos.board.bb_colors[BLACK]);
            BitBoard       occupancy;
            if constexpr ((PT == KNIGHT) || (PT == KING))
                occupancy = (enemies | empty);
            else
                occupancy = ~empty;
            BitBoard bb = pos.board.bb_pieces[PT] & pos.board.bb_colors[US];
            while (bb)
            {
                const Square from = static_cast<Square>(bitscan_forward(&bb));

                BitBoard attacks = 0ULL;

                if constexpr (PT == KNIGHT)
                {
                    attacks = ATTACK_TABLE_KNIGHT[from] & occupancy;
                }
                else if constexpr (PT == BISHOP)
                {
                    attacks = movegen_get_bishop_attacks(from, occupancy);
                }
                else if constexpr (PT == ROOK)
                {
                    attacks = movegen_get_rook_attacks(from, occupancy);
                }
                else if constexpr (PT == QUEEN)
                {
                    attacks = movegen_get_bishop_attacks(from, occupancy)
                            | movegen_get_rook_attacks(from, occupancy);
                }
                else if constexpr (PT == KING)
                {
                    attacks = ATTACK_TABLE_KING[from] & occupancy;
                }

                BitBoard captures = attacks & enemies;
                while (captures)
                {
                    const Square to = static_cast<Square>(bitscan_forward(&captures));
                    move_list->add_move(MOVE_CREATE(from, to, MOVE_CAPTURE));
                }
                if constexpr (T == MovegenType::MOVEGEN_ALL)
                {
                    BitBoard quiets = attacks & empty;
                    while (quiets)
                    {
                        const Square to = static_cast<Square>(bitscan_forward(&quiets));
                        move_list->add_move(MOVE_CREATE(from, to, MOVE_QUIET));
                    }
                }
            }
        }

        template<Color US>
        static void movegen_generate_pseudolegal_moves_castles(const Position& pos,
                                                               MoveList* const move_list) {
            constexpr CastlingRights rights_k = (US == WHITE) ? WKCA : BKCA;
            constexpr CastlingRights rights_q = (US == WHITE) ? WQCA : BQCA;

            if (!(pos.ca_rights & (rights_k | rights_q)))
                return;

            constexpr Color  them   = COLOR_FLIP(US);
            constexpr Rank   rank   = (US == WHITE) ? RANK_1 : RANK_8;
            constexpr Square k_sq   = RF_TO_SQ(rank, FILE_E);
            constexpr Square k_sq_r = static_cast<Square>(k_sq + 1);
            constexpr Square k_sq_l = static_cast<Square>(k_sq - 1);

            const BitBoard occ         = (pos.board.bb_colors[WHITE] | pos.board.bb_colors[BLACK]);
            const BitBoard k_attackers = movegen_get_square_attackers(pos.board, k_sq, them);
            BitBoard       attackers;

            constexpr BitBoard mask_path_k =
              (US == WHITE) ? BITBOARD_MASK_WKCA_PATH : BITBOARD_MASK_BKCA_PATH;
            const BitBoard path_k_empty = MASK64((occ & mask_path_k) == 0);

            constexpr BitBoard mask_path_q =
              (US == WHITE) ? BITBOARD_MASK_WQCA_PATH : BITBOARD_MASK_BQCA_PATH;
            const BitBoard path_q_empty = MASK64((occ & mask_path_q) == 0);

            attackers = k_attackers | movegen_get_square_attackers(pos.board, k_sq_r, them);
            const BitBoard safe_k_ok = MASK64(attackers == 0);

            attackers = k_attackers | movegen_get_square_attackers(pos.board, k_sq_l, them);
            const BitBoard safe_q_ok = MASK64(attackers == 0);

            const BitBoard rights_k_ok = MASK64(pos.ca_rights & rights_k);
            const BitBoard rights_q_ok = MASK64(pos.ca_rights & rights_q);

            const BitBoard rook_bb   = pos.board.bb_pieces[ROOK] & pos.board.bb_colors[US];
            const BitBoard king_bb   = pos.board.bb_pieces[KING] & pos.board.bb_colors[US];
            const BitBoard rook_k_ok = MASK64(rook_bb & BB(RF_TO_SQ(rank, FILE_H)));
            const BitBoard rook_q_ok = MASK64(rook_bb & BB(RF_TO_SQ(rank, FILE_A)));
            const BitBoard king_ok   = MASK64(king_bb & BB(RF_TO_SQ(rank, FILE_E)));

            const BitBoard can_kca = path_k_empty & safe_k_ok & rights_k_ok & rook_k_ok & king_ok;
            const BitBoard can_qca = path_q_empty & safe_q_ok & rights_q_ok & rook_q_ok & king_ok;

            BitBoard dests =
              (BB(RF_TO_SQ(rank, FILE_G)) & can_kca) | (BB(RF_TO_SQ(rank, FILE_C)) & can_qca);

            while (dests)
            {
                const Square   to   = static_cast<Square>(bitscan_forward(&dests));
                const Rank     r    = SQ_TO_RANK(to);
                const File     f    = SQ_TO_FILE(to);
                const Square   from = RF_TO_SQ(r, FILE_E);
                const MoveFlag flag = static_cast<MoveFlag>(2 + ((f == 2) & 1));
                move_list->add_move(MOVE_CREATE(from, to, flag));
            }
        }

        void movegen_init() {
            movegen_init_line_and_ray_bb();

            movegen_init_magic_table_bishop();
            movegen_init_magic_table_rook();

            movegen_init_attack_table_pawn();
            movegen_init_attack_table_knight();
            movegen_init_attack_table_bishop();
            movegen_init_attack_table_rook();
            movegen_init_attack_table_king();
        }

        BitBoard
        movegen_get_square_attackers(const Board& board, const Square sq, const Color attacked_by) {
            const BitBoard occupied   = (board.bb_colors[WHITE] | board.bb_colors[BLACK]);
            const BitBoard op_pieces  = board.bb_colors[attacked_by];
            const BitBoard op_pawns   = board.bb_pieces[PAWN] & op_pieces;
            const BitBoard op_knights = board.bb_pieces[KNIGHT] & op_pieces;
            const BitBoard op_bq   = (board.bb_pieces[BISHOP] | board.bb_pieces[QUEEN]) & op_pieces;
            const BitBoard op_rq   = (board.bb_pieces[ROOK] | board.bb_pieces[QUEEN]) & op_pieces;
            const BitBoard op_king = board.bb_pieces[KING] & op_pieces;
            // clang-format off
            return (movegen_get_bishop_attacks(sq, occupied) & op_bq)
                    | (movegen_get_rook_attacks(sq, occupied) & op_rq)
                    | (ATTACK_TABLE_KNIGHT[sq] & op_knights)
                    | (ATTACK_TABLE_PAWN[COLOR_FLIP(attacked_by)][sq] & op_pawns)
                    | (ATTACK_TABLE_KING[sq] & op_king);
            // clang-format on
        }

        BitBoard movegen_get_pinned_pieces(const Position& pos) {
            BitBoard pinned = 0ULL;

            const Color us = static_cast<Color>(pos.black_to_play);

            const BitBoard our_pieces = pos.board.bb_colors[us];
            const BitBoard op_pieces  = pos.board.bb_colors[COLOR_FLIP(us)];

            const BitBoard op_rq =
              (pos.board.bb_pieces[ROOK] | pos.board.bb_pieces[QUEEN]) & op_pieces;
            const BitBoard op_bq =
              (pos.board.bb_pieces[BISHOP] | pos.board.bb_pieces[QUEEN]) & op_pieces;

            BitBoard diag_attackers = movegen_get_bishop_attacks(pos.king_sq, op_pieces) & op_bq;
            while (diag_attackers)
            {
                const Square   from    = static_cast<Square>(bitscan_forward(&diag_attackers));
                const BitBoard between = LINE_BB[pos.king_sq][from] & our_pieces;
                pinned |= (between && (between & (between - 1)) == 0) ? between : 0ULL;
            }

            BitBoard ortho_attackers = movegen_get_rook_attacks(pos.king_sq, op_pieces) & op_rq;
            while (ortho_attackers)
            {
                const Square   from    = static_cast<Square>(bitscan_forward(&ortho_attackers));
                const BitBoard between = LINE_BB[pos.king_sq][from] & our_pieces;
                pinned |= (between && (between & (between - 1)) == 0) ? between : 0ULL;
            }

            return pinned;
        }

        template<Color US, MovegenType T>
        static void movegen_generate_pseudolegal_moves_color(const Position& pos,
                                                             MoveList*       move_list) {
            movegen_generate_pseudolegal_moves_pawn<US, T>(pos, move_list);
            movegen_generate_pseudolegal_moves_piece<KNIGHT, US, T>(pos, move_list);
            movegen_generate_pseudolegal_moves_piece<BISHOP, US, T>(pos, move_list);
            movegen_generate_pseudolegal_moves_piece<ROOK, US, T>(pos, move_list);
            movegen_generate_pseudolegal_moves_piece<QUEEN, US, T>(pos, move_list);
            movegen_generate_pseudolegal_moves_piece<KING, US, T>(pos, move_list);
            if constexpr (T == MovegenType::MOVEGEN_ALL)
                movegen_generate_pseudolegal_moves_castles<US>(pos, move_list);
        }

        template<MovegenType T>
        void movegen_generate_pseudolegal_moves(const Position& pos, MoveList* const move_list) {
            if (pos.black_to_play)
                movegen_generate_pseudolegal_moves_color<BLACK, T>(pos, move_list);
            else
                movegen_generate_pseudolegal_moves_color<WHITE, T>(pos, move_list);
        }

        template void movegen_generate_pseudolegal_moves<MovegenType::MOVEGEN_ALL>(const Position&,
                                                                                   MoveList*);

        template void
        movegen_generate_pseudolegal_moves<MovegenType::MOVEGEN_CAPTURES>(const Position&,
                                                                          MoveList*);

        inline BitBoard ray(const Square sq1, const Square sq2) { return RAY_BB[sq1][sq2]; }

        inline BitBoard path_between(const Square sq1, const Square sq2) {
            return LINE_BB[sq1][sq2];
        }
    }

}
