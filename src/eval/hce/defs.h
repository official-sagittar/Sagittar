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
        S(98, 164),
        S(425, 436),
        S(436, 441),
        S(584, 721),
        S(1284, 1210),
        0
    };

    using PSQT = std::array<i32, 64>;

    constexpr std::array<PSQT, 6> PSQT_SCORES = []() {
        std::array<PSQT, 6> table{};

        table[PieceType::PAWN] = {
            S(0, 0),        S(0, 0),        S(0, 0),        S(0, 0),        S(0, 0),        S(0, 0),        S(0, 0),        S(0, 0),
            S(94, 251),     S(130, 235),    S(83, 243),     S(122, 170),    S(99, 165),     S(75, 184),     S(-10, 243),    S(-47, 262),
            S(-6, 163),     S(3, 179),      S(48, 131),     S(55, 99),      S(60, 84),      S(93, 68),      S(68, 133),     S(13, 125),
            S(-29, 62),     S(1, 48),       S(4, 24),       S(10, 9),       S(37, -3),      S(30, 2),       S(29, 27),      S(2, 25),
            S(-34, 23),     S(-9, 24),      S(2, -1),       S(19, -9),      S(22, -10),     S(17, -5),      S(17, 9),       S(-10, -5),
            S(-33, 13),     S(-2, 19),      S(1, 2),        S(5, 10),       S(27, 3),       S(18, 3),       S(46, 5),       S(0, -10),
            S(-28, 16),     S(-1, 24),      S(-1, 9),       S(1, 17),       S(16, 22),      S(40, 6),       S(61, 3),       S(-6, -11),
            S(0, 0),        S(0, 0),        S(0, 0),        S(0, 0),        S(0, 0),        S(0, 0),        S(0, 0),        S(0, 0),
        };

        table[PieceType::KNIGHT] = {
            S(-194, -52),   S(-143, -3),    S(-75, 12),     S(-34, 0),      S(22, 3),       S(-62, -27),    S(-67, -10),    S(-116, -87),
            S(-15, -5),     S(4, 14),       S(37, 12),      S(61, 11),      S(36, 1),       S(126, -22),    S(9, 8),        S(50, -31),
            S(0, 10),       S(39, 17),      S(53, 45),      S(70, 43),      S(124, 19),     S(125, 15),     S(75, -1),      S(46, -8),
            S(2, 23),       S(12, 36),      S(39, 55),      S(73, 57),      S(47, 56),      S(77, 51),      S(30, 36),      S(57, 12),
            S(-9, 29),      S(1, 25),       S(18, 57),      S(31, 54),      S(41, 59),      S(35, 46),      S(40, 28),      S(11, 24),
            S(-38, 6),      S(-11, 13),     S(3, 25),       S(13, 47),      S(36, 42),      S(18, 17),      S(19, 10),      S(-5, 13),
            S(-44, 3),      S(-26, 13),     S(-12, 15),     S(16, 16),      S(16, 16),      S(10, 9),       S(-7, 5),       S(-9, 22),
            S(-98, 4),      S(-20, -7),     S(-31, 5),      S(-5, 15),      S(0, 16),       S(0, 1),        S(-17, 1),      S(-53, -2),
        };

        table[PieceType::BISHOP] = {
            S(-22, 27),     S(-60, 34),     S(-56, 29),     S(-112, 44),    S(-90, 36),     S(-71, 22),     S(-29, 17),     S(-61, 17),
            S(-4, 4),       S(20, 18),      S(3, 23),       S(-13, 27),     S(23, 13),      S(19, 15),      S(16, 24),      S(15, 0),
            S(7, 36),       S(33, 26),      S(30, 35),      S(54, 20),      S(36, 28),      S(89, 29),      S(61, 21),      S(50, 27),
            S(-4, 27),      S(19, 39),      S(37, 35),      S(59, 52),      S(52, 39),      S(43, 40),      S(19, 33),      S(2, 27),
            S(5, 18),       S(2, 39),       S(17, 48),      S(53, 43),      S(47, 43),      S(12, 42),      S(8, 33),       S(25, 7),
            S(10, 24),      S(30, 36),      S(27, 38),      S(29, 43),      S(31, 48),      S(32, 37),      S(29, 28),      S(32, 14),
            S(28, 28),      S(27, 13),      S(39, 11),      S(15, 30),      S(32, 25),      S(42, 17),      S(49, 23),      S(32, 7),
            S(9, 12),       S(48, 26),      S(19, 22),      S(8, 22),       S(16, 21),      S(10, 36),      S(39, 5),       S(27, -6),
        };

        table[PieceType::ROOK] = {
            S(51, 127),     S(29, 139),     S(27, 151),     S(28, 145),     S(52, 134),     S(78, 124),     S(71, 125),     S(106, 114),
            S(28, 131),     S(26, 147),     S(50, 151),     S(79, 136),     S(61, 137),     S(102, 120),    S(96, 114),     S(137, 94),
            S(8, 133),      S(46, 134),     S(43, 135),     S(50, 132),     S(91, 113),     S(105, 105),    S(162, 93),     S(125, 87),
            S(1, 137),      S(20, 132),     S(20, 144),     S(29, 135),     S(40, 116),     S(49, 110),     S(72, 105),     S(63, 101),
            S(-15, 126),    S(-13, 130),    S(1, 131),      S(13, 127),     S(13, 123),     S(1, 122),      S(42, 104),     S(21, 100),
            S(-19, 118),    S(-8, 116),     S(5, 114),      S(7, 117),      S(18, 110),     S(22, 99),      S(63, 75),      S(28, 78),
            S(-20, 107),    S(-5, 111),     S(22, 110),     S(25, 111),     S(32, 99),      S(33, 93),      S(54, 81),      S(6, 90),
            S(5, 107),      S(17, 107),     S(33, 113),     S(44, 106),     S(50, 97),      S(39, 101),     S(50, 90),      S(9, 89),
        };

        table[PieceType::QUEEN] = {
            S(64, 181),     S(52, 205),     S(97, 220),     S(126, 212),    S(131, 211),    S(156, 189),    S(168, 146),    S(120, 174),
            S(100, 158),    S(64, 206),     S(68, 246),     S(69, 258),     S(82, 275),     S(127, 233),    S(109, 210),    S(183, 185),
            S(105, 166),    S(96, 185),     S(95, 229),     S(104, 247),    S(120, 257),    S(188, 230),    S(194, 184),    S(198, 179),
            S(84, 176),     S(91, 196),     S(88, 217),     S(88, 247),     S(92, 267),     S(112, 249),    S(124, 239),    S(129, 216),
            S(94, 164),     S(83, 203),     S(90, 209),     S(99, 236),     S(100, 232),    S(98, 227),     S(110, 212),    S(125, 199),
            S(95, 147),     S(103, 173),    S(102, 190),    S(100, 191),    S(106, 198),    S(115, 193),    S(129, 172),    S(127, 151),
            S(107, 133),    S(106, 140),    S(117, 143),    S(128, 143),    S(125, 151),    S(137, 119),    S(143, 82),     S(161, 52),
            S(104, 124),    S(112, 128),    S(123, 127),    S(127, 146),    S(130, 122),    S(110, 129),    S(134, 101),    S(113, 98),
        };

        table[PieceType::KING] = {
            S(-52, -118),   S(-3, -65),     S(11, -45),     S(-60, -2),     S(-55, -11),    S(-16, -2),     S(28, -8),      S(19, -121),
            S(-47, -24),    S(-35, 28),     S(-78, 41),     S(11, 27),      S(-19, 50),     S(-13, 66),     S(11, 57),      S(-2, 10),
            S(-97, 3),      S(17, 37),      S(-66, 61),     S(-78, 74),     S(-42, 76),     S(46, 69),      S(20, 66),      S(-22, 22),
            S(-79, -8),     S(-96, 42),     S(-111, 67),    S(-164, 83),    S(-156, 84),    S(-112, 77),    S(-104, 64),    S(-137, 27),
            S(-78, -23),    S(-88, 21),     S(-129, 54),    S(-172, 76),    S(-174, 76),    S(-122, 57),    S(-125, 40),    S(-158, 17),
            S(-25, -37),    S(0, -4),       S(-79, 26),     S(-101, 44),    S(-94, 44),     S(-87, 31),     S(-27, 5),      S(-51, -13),
            S(99, -66),     S(37, -27),     S(19, -10),     S(-25, 4),      S(-31, 10),     S(-8, -2),      S(57, -29),     S(72, -55),
            S(89, -115),    S(136, -92),    S(102, -63),    S(-34, -36),    S(48, -63),     S(3, -40),      S(104, -80),    S(101, -120),
        };

        return table;
    }();
    // clang-format on

    constexpr std::array<std::array<i32, 28>, 4> MOBILITY_BONUS = []() {
        std::array<std::array<i32, 28>, 4> table{};

        table[0] = {
          S(-48, -63), S(-21, -17), S(-5, 10), S(3, 22),  S(11, 33),
          S(19, 44),   S(29, 41),   S(39, 35), S(48, 20),
        };

        table[1] = {
          S(-43, -59), S(-27, -33), S(-11, -10), S(0, 8),   S(12, 21), S(19, 37), S(26, 42),
          S(29, 45),   S(31, 50),   S(38, 44),   S(49, 39), S(58, 35), S(60, 44), S(57, 22),
        };

        table[2] = {
          S(-53, -10), S(-35, 14), S(-29, 16), S(-20, 21), S(-20, 32),
          S(-8, 36),   S(-1, 45),  S(10, 47),  S(22, 51),  S(30, 57),
          S(39, 59),   S(41, 65),  S(48, 69),  S(55, 67),  S(28, 73),
        };

        table[3] = {
          S(19, -13),  S(35, -58),  S(31, -23),  S(37, 11),   S(40, 35),   S(47, 43),   S(52, 66),
          S(55, 84),   S(60, 94),   S(63, 100),  S(66, 110),  S(70, 120),  S(73, 124),  S(72, 136),
          S(74, 140),  S(79, 145),  S(80, 153),  S(84, 153),  S(96, 150),  S(109, 146), S(118, 147),
          S(143, 141), S(135, 141), S(127, 137), S(125, 136), S(100, 118), S(55, 75),   S(39, 59),
        };

        return table;
    }();

    constexpr i32 BISHOP_PAIR   = S(28, 94);
    constexpr i32 PAWN_DOUBLED  = S(-33, -57);
    constexpr i32 PAWN_DOUBLED2 = S(-9, -34);
    constexpr i32 TEMPO_BONUS   = S(15, 3);
}
