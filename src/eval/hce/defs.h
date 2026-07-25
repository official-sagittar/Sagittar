#pragma once

#include "commons/pch.h"
#include "core/movegen.h"
#include "core/position.h"
#include "core/types.h"

namespace sagittar::eval::hce {

    enum GamePhase : u8 {
        MG,
        EG
    };

    constexpr std::array<i32, 6> PHASE_WEIGHTS = {0, 1, 1, 2, 4, 0};
    constexpr i32                TOTAL_PHASE   = 24;

    constexpr i32 S(const i32 mg, const i32 eg) {
        return static_cast<i32>(static_cast<u32>(eg) << 16) + mg;
    }
    constexpr i32 mg_score(const i32 score) { return static_cast<i16>(score); }
    constexpr i32 eg_score(const i32 score) { return static_cast<i16>((score + 0x8000) >> 16); }

    constexpr Score scale_eval(const Score mg, const Score eg, const i32 phase) {
        return static_cast<Score>(((mg * (256 - phase)) + (eg * phase)) / 256);
    }

    constexpr i32 std_phase(const i32 phase) {
        return std::clamp(((phase * 256 + (TOTAL_PHASE / 2)) / TOTAL_PHASE), 0, 256);
    }

    inline i32 pos_phase(const Position& pos) {
        i32 phase = TOTAL_PHASE;

        for (int pt = PieceType::PAWN; pt <= PieceType::KING; pt++)
        {
            phase -= pos.pieceCount(static_cast<PieceType>(pt)) * PHASE_WEIGHTS[pt];
        }

        phase = std_phase(phase);

        return phase;
    }

    template<Color US>
    int mobility(const Position& pos, const PieceType pt, const Square sq) {
        constexpr Color them               = colorFlip(US);
        const auto      pieces_us          = pos.pieces(US);
        const auto      pieces_them        = pos.pieces(them);
        const auto      occupied           = pieces_us | pieces_them;
        const auto      empty              = ~occupied;
        const auto      enemy_pawns        = pos.pieces(them, PieceType::PAWN);
        const auto      enemy_king         = pos.pieces(them, PieceType::KING);
        const auto      enemies            = pieces_them ^ enemy_king;
        const auto      enemy_pawn_attacks = (US == Color::WHITE)
                                             ? (shift<Direction::SOUTH_EAST>(enemy_pawns)
                                           | shift<Direction::SOUTH_WEST>(enemy_pawns))
                                             : (shift<Direction::NORTH_EAST>(enemy_pawns)
                                           | shift<Direction::NORTH_WEST>(enemy_pawns));
        BitBoard        attks{};
        if (pt == PieceType::KNIGHT)
        {
            attks = attacks<PieceType::KNIGHT>(sq, (enemies | empty));
        }
        else
        {
            const auto all_attks = attacks(pt, sq, occupied);
            attks                = (all_attks & enemies) | (all_attks & empty);
        }

        const auto mobility = attks ^ (attks & enemy_pawn_attacks);

        assert(mobility.count() >= 0 && mobility.count() <= 27);
        return mobility.count();
    }

    // clang-format off
    constexpr std::array<i32, 6> PIECE_SCORES = {
        S(95, 145),
        S(415, 395),
        S(420, 398),
        S(570, 662),
        S(1218, 1131),
        0
    };

    using PSQT = std::array<i32, 64>;

    constexpr std::array<PSQT, 6> PSQT_SCORES = []() {
        std::array<PSQT, 6> table{};

        table[PieceType::PAWN] = {
            S(0, 0),        S(0, 0),        S(0, 0),        S(0, 0),        S(0, 0),        S(0, 0),        S(0, 0),        S(0, 0),
            S(98, 235),     S(131, 218),    S(89, 225),     S(124, 159),    S(103, 156),    S(84, 169),     S(-1, 224),     S(-39, 244),
            S(-4, 157),     S(5, 171),      S(47, 124),     S(54, 96),      S(57, 84),      S(90, 65),      S(69, 125),     S(14, 122),
            S(-26, 65),     S(2, 49),       S(1, 24),       S(7, 12),       S(33, -1),      S(26, 3),       S(29, 29),      S(5, 31),
            S(-31, 28),     S(-7, 28),      S(-1, 1),       S(17, -3),      S(20, -6),      S(13, -4),      S(16, 13),      S(-8, 2),
            S(-31, 20),     S(-3, 22),      S(-5, 0),       S(3, 14),       S(23, 5),       S(8, 1),        S(41, 9),       S(3, -2),
            S(-25, 25),     S(0, 29),       S(-4, 8),       S(2, 21),       S(15, 26),      S(36, 7),       S(59, 8),       S(-3, -2),
            S(0, 0),        S(0, 0),        S(0, 0),        S(0, 0),        S(0, 0),        S(0, 0),        S(0, 0),        S(0, 0),
        };

        table[PieceType::KNIGHT] = {
            S(-183, -40),   S(-125, -4),    S(-63, 9),      S(-27, -1),     S(28, -1),      S(-60, -23),    S(-48, -12),    S(-107, -77),
            S(-17, -4),     S(5, 10),       S(37, 11),      S(57, 9),       S(34, 0),       S(122, -21),    S(8, 6),        S(44, -29),
            S(-2, 9),       S(37, 14),      S(52, 39),      S(69, 38),      S(120, 16),     S(119, 13),     S(72, -3),      S(42, -9),
            S(3, 17),       S(11, 33),      S(37, 51),      S(72, 52),      S(46, 53),      S(74, 46),      S(28, 33),      S(55, 10),
            S(-11, 28),     S(-1, 24),      S(17, 53),      S(30, 50),      S(40, 54),      S(34, 41),      S(37, 25),      S(9, 20),
            S(-39, 6),      S(-12, 11),     S(2, 24),       S(11, 43),      S(33, 37),      S(16, 14),      S(17, 8),       S(-6, 12),
            S(-46, 5),      S(-27, 13),     S(-13, 12),     S(15, 13),      S(15, 14),      S(8, 7),        S(-8, 3),       S(-11, 20),
            S(-99, 3),      S(-23, -3),     S(-35, 5),      S(-8, 14),      S(-4, 12),      S(-2, -2),      S(-19, 3),      S(-55, -6),
        };

        table[PieceType::BISHOP] = {
            S(-24, 28),     S(-51, 30),     S(-52, 29),     S(-95, 40),     S(-77, 32),     S(-63, 20),     S(-24, 15),     S(-57, 18),
            S(-5, 3),       S(20, 17),      S(4, 22),       S(-11, 25),     S(24, 12),      S(18, 14),      S(17, 22),      S(10, 1),
            S(5, 34),       S(32, 24),      S(29, 33),      S(53, 16),      S(35, 24),      S(85, 26),      S(56, 20),      S(46, 24),
            S(-4, 27),      S(17, 37),      S(36, 33),      S(58, 48),      S(52, 36),      S(42, 37),      S(17, 31),      S(0, 26),
            S(1, 17),       S(1, 37),       S(14, 44),      S(51, 38),      S(45, 39),      S(12, 38),      S(7, 31),       S(23, 7),
            S(8, 22),       S(27, 33),      S(26, 35),      S(26, 38),      S(30, 43),      S(30, 34),      S(27, 24),      S(31, 13),
            S(24, 28),      S(26, 13),      S(36, 9),       S(14, 27),      S(29, 23),      S(41, 14),      S(48, 23),      S(31, 6),
            S(6, 14),       S(43, 23),      S(17, 19),      S(3, 20),       S(14, 19),      S(7, 33),       S(36, 5),       S(25, -5),
        };

        table[PieceType::ROOK] = {
            S(51, 113),     S(34, 123),     S(36, 133),     S(36, 128),     S(57, 117),     S(74, 112),     S(69, 112),     S(103, 100),
            S(30, 117),     S(30, 132),     S(54, 136),     S(80, 122),     S(65, 122),     S(103, 107),    S(94, 103),     S(131, 83),
            S(10, 121),     S(47, 121),     S(45, 123),     S(53, 119),     S(91, 102),     S(99, 95),      S(152, 85),     S(118, 78),
            S(2, 124),      S(21, 119),     S(21, 131),     S(31, 123),     S(42, 106),     S(45, 101),     S(66, 95),      S(61, 91),
            S(-13, 113),    S(-13, 118),    S(1, 119),      S(15, 116),     S(15, 112),     S(0, 111),      S(38, 94),      S(21, 89),
            S(-18, 107),    S(-8, 104),     S(4, 103),      S(7, 107),      S(18, 99),      S(20, 89),      S(61, 64),      S(28, 69),
            S(-19, 98),     S(-5, 100),     S(21, 99),      S(25, 101),     S(31, 89),      S(32, 83),      S(52, 72),      S(7, 81),
            S(5, 99),       S(15, 97),      S(32, 103),     S(42, 97),      S(49, 88),      S(37, 92),      S(47, 80),      S(8, 84),
        };

        table[PieceType::QUEEN] = {
            S(41, 135),     S(37, 153),     S(83, 165),     S(106, 163),    S(115, 159),    S(129, 141),    S(134, 106),    S(94, 126),
            S(73, 116),     S(40, 163),     S(53, 189),     S(56, 196),     S(70, 211),     S(108, 177),    S(85, 161),     S(154, 133),
            S(80, 119),     S(71, 140),     S(78, 168),     S(83, 197),     S(99, 200),     S(162, 179),    S(161, 141),    S(166, 132),
            S(60, 131),     S(63, 156),     S(64, 171),     S(66, 195),     S(71, 213),     S(91, 194),     S(95, 192),     S(104, 166),
            S(66, 123),     S(56, 161),     S(61, 168),     S(73, 192),     S(75, 183),     S(70, 182),     S(84, 165),     S(98, 150),
            S(67, 105),     S(75, 129),     S(72, 152),     S(71, 152),     S(78, 156),     S(87, 152),     S(101, 125),    S(99, 106),
            S(76, 97),      S(77, 101),     S(89, 102),     S(100, 103),    S(96, 110),     S(107, 79),     S(112, 48),     S(123, 24),
            S(74, 79),      S(78, 91),      S(90, 93),      S(99, 99),      S(96, 90),      S(75, 92),      S(88, 69),      S(68, 64),
        };

        table[PieceType::KING] = {
            S(-60, -108),   S(-6, -64),     S(2, -44),      S(-51, -5),     S(-59, -11),    S(-24, -1),     S(19, -4),      S(11, -101),
            S(-29, -28),    S(-31, 24),     S(-68, 36),     S(-2, 26),      S(-20, 45),     S(-15, 60),     S(2, 53),       S(-8, 11),
            S(-78, 0),      S(12, 34),      S(-59, 56),     S(-65, 67),     S(-37, 71),     S(32, 66),      S(16, 62),      S(-25, 20),
            S(-72, -8),     S(-87, 38),     S(-96, 62),     S(-138, 76),    S(-131, 77),    S(-101, 72),    S(-93, 59),     S(-121, 24),
            S(-78, -22),    S(-83, 19),     S(-125, 51),    S(-162, 71),    S(-166, 71),    S(-121, 54),    S(-121, 38),    S(-147, 15),
            S(-29, -33),    S(-6, -3),      S(-81, 25),     S(-102, 42),    S(-92, 41),     S(-89, 29),     S(-31, 4),      S(-53, -10),
            S(90, -59),     S(29, -24),     S(12, -9),      S(-31, 5),      S(-37, 10),     S(-14, -1),     S(51, -26),     S(63, -51),
            S(77, -104),    S(125, -86),    S(91, -57),     S(-43, -32),    S(38, -57),     S(-7, -35),     S(93, -72),     S(89, -112),
        };

        return table;
    }();
    // clang-format on

    constexpr std::array<std::array<i32, 28>, 4> MOBILITY_BONUS = []() {
        std::array<std::array<i32, 28>, 4> table{};

        // Knight (0-8)
        table[0] = {
          S(-50, -55), S(-22, -26), S(-7, 0),  S(2, 11), S(10, 21),
          S(18, 31),   S(27, 28),   S(36, 23), S(45, 8),
        };

        // Bishop (0-13)
        table[1] = {
          S(-43, -55), S(-25, -40), S(-9, -20), S(2, -2),  S(13, 11), S(20, 26), S(27, 31),
          S(31, 33),   S(33, 39),   S(39, 33),  S(48, 30), S(54, 28), S(43, 39), S(34, 19),
        };

        // Rook (0-14)
        table[2] = {
          S(-47, -40), S(-31, -16), S(-26, -12), S(-18, -8), S(-18, 2),
          S(-7, 7),    S(-1, 15),   S(9, 17),    S(20, 21),  S(28, 27),
          S(35, 28),   S(37, 34),   S(43, 39),   S(46, 36),  S(19, 45),
        };

        // Queen (0-27)
        table[3] = {
          S(-18, -10), S(-6, -37), S(-8, -33), S(0, -21), S(4, -7),  S(11, -2), S(17, 20),
          S(19, 39),   S(24, 48),  S(27, 54),  S(31, 62), S(36, 67), S(40, 69), S(41, 74),
          S(45, 75),   S(51, 76),  S(55, 79),  S(59, 79), S(67, 79), S(72, 79), S(75, 80),
          S(80, 80),   S(75, 78),  S(70, 76),  S(66, 72), S(52, 62), S(25, 35), S(17, 26),
        };

        return table;
    }();

    constexpr i32 BISHOP_PAIR = S(30, 84);
    constexpr i32 TEMPO_BONUS = S(15, 3);
}
