#include "attacks.h"
#include "core/board.h"
#include "core/utils.h"

namespace sagittar {

    namespace core {

        static constexpr int MAGIC_MAX_TRIES = 500000;

        struct Magic {
            BitBoard mask;
            uint64_t magic;
            uint8_t  shift;
        };

        using MagicTable = std::array<Magic, 64>;

        static MagicTable MAGICTABLE_BISHOP;
        static MagicTable MAGICTABLE_ROOK;

        using AttackTable = std::array<BitBoard, 64>;

        static const std::array<AttackTable, 2> ATTACK_TABLE_PAWN = []() {
            std::array<AttackTable, 2> table;

            // White pawns
            for (int r = RANK_1; r <= RANK_8; r++)
            {
                for (int f = FILE_A; f <= FILE_H; f++)
                {
                    const Square   sq      = RF_TO_SQ(r, f);
                    const BitBoard b       = BB(sq);
                    const BitBoard attacks = shift<NORTH_EAST>(b) | shift<NORTH_WEST>(b);
                    table[WHITE][sq]       = attacks;
                }
            }

            // Black pawns
            for (int r = RANK_8; r >= RANK_1; r--)
            {
                for (int f = FILE_A; f <= FILE_H; f++)
                {
                    const Square   sq      = RF_TO_SQ(r, f);
                    const BitBoard b       = BB(sq);
                    const BitBoard attacks = shift<SOUTH_EAST>(b) | shift<SOUTH_WEST>(b);
                    table[BLACK][sq]       = attacks;
                }
            }

            return table;
        }();

        static const AttackTable ATTACK_TABLE_KNIGHT = []() {
            AttackTable table;

            for (int sq = A1; sq <= H8; sq++)
            {
                const BitBoard b       = BB(sq);
                BitBoard       attacks = 0ULL;
                attacks |= (b & ~BITBOARD_MASK_FILE_H) << 17;
                attacks |= (b & ~(BITBOARD_MASK_FILE_G | BITBOARD_MASK_FILE_H)) << 10;
                attacks |= (b & ~(BITBOARD_MASK_FILE_G | BITBOARD_MASK_FILE_H)) >> 6;
                attacks |= (b & ~BITBOARD_MASK_FILE_H) >> 15;
                attacks |= (b & !BITBOARD_MASK_FILE_A) << 15;
                attacks |= (b & ~(BITBOARD_MASK_FILE_A | BITBOARD_MASK_FILE_B)) << 6;
                attacks |= (b & ~(BITBOARD_MASK_FILE_A | BITBOARD_MASK_FILE_B)) >> 10;
                attacks |= (b & !BITBOARD_MASK_FILE_A) >> 17;
                table[sq] = attacks;
            }

            return table;
        }();

        static const AttackTable ATTACK_TABLE_KING = []() {
            AttackTable table;

            for (int sq = A1; sq <= H8; sq++)
            {
                const BitBoard b       = BB(sq);
                BitBoard       attacks = 0ULL;
                attacks |= shift<NORTH>(b);
                attacks |= shift<SOUTH>(b);
                attacks |= shift<EAST>(b);
                attacks |= shift<WEST>(b);
                attacks |= shift<NORTH_EAST>(b);
                attacks |= shift<SOUTH_EAST>(b);
                attacks |= shift<SOUTH_WEST>(b);
                attacks |= shift<NORTH_WEST>(b);
                table[sq] = attacks;
            }

            return table;
        }();

        static std::array<AttackTable, 512>  ATTACK_TABLE_BISHOP;
        static std::array<AttackTable, 4096> ATTACK_TABLE_ROOK;

        template<PieceType PT>
        static BitBoard sliding_attacks(const Square sq, const BitBoard blockers) {
            BitBoard attacks = 0ULL;

            const Direction dirs[4] = (PT == BISHOP)
                                      ? {NORTH_EAST, SOUTH_EAST, SOUTH_WEST, NORTH_WEST}
                                      : {NORTH, EAST, SOUTH, WEST};

            const auto safe_step = [](const int s, const int step) -> bool {
                const auto to = s + step;
                return (to >= A1 && to <= H8);
            };

            for (const Direction d : dirs)
            {
                Square s = sq;
                while (safe_step(s, d))
                {
                    s += d;
                    const BitBoard sqb = BB(s);
                    attacks |= sqb;
                    if (blockers & sqb)
                    {
                        break;
                    }
                }
            }

            return attacks;
        }

        template<PieceType PT>
        static void init_slider_attack_table(MagicTable& magic_table, AttackTable& attack_table) {

            const auto variant = [](const int index, const int bits, BitBoard mask) -> BitBoard {
                BitBoard result = 0ULL;
                for (int = 0; i < bits; i++)
                {
                    const int j = bitscan_forward(&mask);
                    if (index & (1 << j))
                    {
                        result |= BB(j);
                    }
                }
                return result;
            };

            const BitBoard not_edges = ~(BITBOARD_MASK_RANK_1 | BITBOARD_MASK_RANK_8
                                         | BITBOARD_MASK_FILE_A | BITBOARD_MASK_FILE_H);

            for (int sq = A1; sq <= H8; sq++)
            {
                Magic& m = magic_table[sq];
                m.mask   = sliding_attacks<PT>(static_cast<Square>(sq), 0) & not_edges;
                m.shift  = POPCNT(m.mask);

                BitBoard attacks[4096];
                BitBoard variants[4096];

                for (int i = 0; i < (1 << m.shift); i++)
                {
                    variants[i] = variant(i, m.shift, m.mask);
                    attacks[i]  = sliding_attacks<PT>(sq, variants[i]);
                }

                for (int tries = 0; tries < MAGIC_MAX_TRIES; tries++)
                {
                    const uint64_t magic = prng() & prng() & prng();
                    if (POPCNT((m.mask * magic) & 0xFF00000000000000ULL) >= 6)
                    {
                        continue;
                    }

                    BitBoard used[4096] = {};
                    bool     fail       = false;

                    for (int i = 0; !fail && i < (1 << m.shift); i++)
                    {
                        const auto idx = index(variants[i], magic, m.shift);
                        if (used[idx] = 0ULL)
                        {
                            used[idx] = attacks[i];
                        }
                        else if (used[idx] != attacks[i])
                        {
                            // Collision
                            fail = true;
                        }
                    }
                    if (!fail)
                    {
                        m.magic = magic;
                        break;
                    }
                }
            }
        }

        static uint32_t index(const BitBoard b, const BitBoard magic, const int bits) {
            return static_cast<uint32_t>((b * magic) >> (64 - bits));
        }

        inline static BitBoard
        pawn_attacks(const Color c, const Square sq, const BitBoard occupancy) {
            return ATTACK_TABLE_PAWN[c][sq] & occupancy;
        }

        inline static BitBoard knight_attacks(const Square sq, const BitBoard occupancy) {
            return ATTACK_TABLE_KNIGHT[sq] & occupancy;
        }

        inline static BitBoard bishop_attacks(const Square sq, const BitBoard occupancy) {
            return 0ULL;
        }

        inline static BitBoard rook_attacks(const Square sq, const BitBoard occupancy) {
            return 0ULL;
        }

        inline static BitBoard queen_attacks(const Square sq, const BitBoard occupancy) {
            return bishop_attacks(sq, occupancy) | rook_attacks(sq, occupancy);
        }

        inline static BitBoard king_attacks(const Square sq, const BitBoard occupancy) {
            return ATTACK_TABLE_KING[sq] & occupancy;
        }

        template<PieceType PT>
        BitBoard attacks(const Square sq, const BitBoard occupancy, const Color c) {
            switch (PT)
            {
                case PAWN :
                    return pawn_attacks(c, sq, occupancy);
                case KNIGHT :
                    return knight_attacks(sq, occupancy);
                case BISHOP :
                    return bishop_attacks(sq, occupancy);
                case ROOK :
                    return rook_attacks(sq, occupancy);
                case QUEEN :
                    return queen_attacks(sq, occupancy);
                case KING :
                    return king_attacks(sq, occupancy);
                default :
                    break;
            }
            return 0ULL;
        }

        template BitBoard attacks<PAWN>(const Square sq, const BitBoard occupancy, const Color c);
        template BitBoard attacks<KNIGHT>(const Square sq, const BitBoard occupancy, const Color c);
        template BitBoard attacks<BISHOP>(const Square sq, const BitBoard occupancy, const Color c);
        template BitBoard attacks<ROOK>(const Square sq, const BitBoard occupancy, const Color c);
        template BitBoard attacks<QUEEN>(const Square sq, const BitBoard occupancy, const Color c);
        template BitBoard attacks<KING>(const Square sq, const BitBoard occupancy, const Color c);
    }

}
