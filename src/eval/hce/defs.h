#pragma once

#include "commons/pch.h"
#include "core/movegen.h"
#include "core/position.h"
#include "core/types.h"

namespace sagittar::eval::hce {

    enum GamePhase : U8 {
        MG,
        EG
    };

    constexpr std::array<I32, 6> k_PHASE_WEIGHTS = {0, 1, 1, 2, 4, 0};
    constexpr I32                k_TOTAL_PHASE   = 24;

    constexpr I32 s(const I32 mg, const I32 eg) {
        return static_cast<I32>(static_cast<U32>(eg) << 16) + mg;
    }
    constexpr I32 mgScore(const I32 score) { return static_cast<I16>(score); }
    constexpr I32 egScore(const I32 score) { return static_cast<I16>((score + 0x8000) >> 16); }

    constexpr Score scaleEval(const Score mg, const Score eg, const I32 phase) {
        return static_cast<Score>(((mg * (256 - phase)) + (eg * phase)) / 256);
    }

    constexpr I32 stdPhase(const I32 phase) {
        return std::clamp(((phase * 256 + (k_TOTAL_PHASE / 2)) / k_TOTAL_PHASE), 0, 256);
    }

    inline I32 posPhase(const Position& pos) {
        I32 phase = k_TOTAL_PHASE;

        for (int pt = PieceType::PAWN; pt <= PieceType::KING; pt++)
        {
            phase -= pos.pieceCount(static_cast<PieceType>(pt)) * k_PHASE_WEIGHTS[pt];
        }

        phase = stdPhase(phase);

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
    constexpr std::array<I32, 6> k_PIECE_SCORES = {
        s(95, 145),
        s(415, 395),
        s(420, 398),
        s(570, 662),
        s(1218, 1131),
        0
    };

    using PSQT = std::array<I32, 64>;

    constexpr std::array<PSQT, 6> k_PSQT_SCORES = []() {
        std::array<PSQT, 6> table{};

        table[PieceType::PAWN] = {
            s(0, 0),        s(0, 0),        s(0, 0),        s(0, 0),        s(0, 0),        s(0, 0),        s(0, 0),        s(0, 0),
            s(98, 235),     s(131, 218),    s(89, 225),     s(124, 159),    s(103, 156),    s(84, 169),     s(-1, 224),     s(-39, 244),
            s(-4, 157),     s(5, 171),      s(47, 124),     s(54, 96),      s(57, 84),      s(90, 65),      s(69, 125),     s(14, 122),
            s(-26, 65),     s(2, 49),       s(1, 24),       s(7, 12),       s(33, -1),      s(26, 3),       s(29, 29),      s(5, 31),
            s(-31, 28),     s(-7, 28),      s(-1, 1),       s(17, -3),      s(20, -6),      s(13, -4),      s(16, 13),      s(-8, 2),
            s(-31, 20),     s(-3, 22),      s(-5, 0),       s(3, 14),       s(23, 5),       s(8, 1),        s(41, 9),       s(3, -2),
            s(-25, 25),     s(0, 29),       s(-4, 8),       s(2, 21),       s(15, 26),      s(36, 7),       s(59, 8),       s(-3, -2),
            s(0, 0),        s(0, 0),        s(0, 0),        s(0, 0),        s(0, 0),        s(0, 0),        s(0, 0),        s(0, 0),
        };

        table[PieceType::KNIGHT] = {
            s(-183, -40),   s(-125, -4),    s(-63, 9),      s(-27, -1),     s(28, -1),      s(-60, -23),    s(-48, -12),    s(-107, -77),
            s(-17, -4),     s(5, 10),       s(37, 11),      s(57, 9),       s(34, 0),       s(122, -21),    s(8, 6),        s(44, -29),
            s(-2, 9),       s(37, 14),      s(52, 39),      s(69, 38),      s(120, 16),     s(119, 13),     s(72, -3),      s(42, -9),
            s(3, 17),       s(11, 33),      s(37, 51),      s(72, 52),      s(46, 53),      s(74, 46),      s(28, 33),      s(55, 10),
            s(-11, 28),     s(-1, 24),      s(17, 53),      s(30, 50),      s(40, 54),      s(34, 41),      s(37, 25),      s(9, 20),
            s(-39, 6),      s(-12, 11),     s(2, 24),       s(11, 43),      s(33, 37),      s(16, 14),      s(17, 8),       s(-6, 12),
            s(-46, 5),      s(-27, 13),     s(-13, 12),     s(15, 13),      s(15, 14),      s(8, 7),        s(-8, 3),       s(-11, 20),
            s(-99, 3),      s(-23, -3),     s(-35, 5),      s(-8, 14),      s(-4, 12),      s(-2, -2),      s(-19, 3),      s(-55, -6),
        };

        table[PieceType::BISHOP] = {
            s(-24, 28),     s(-51, 30),     s(-52, 29),     s(-95, 40),     s(-77, 32),     s(-63, 20),     s(-24, 15),     s(-57, 18),
            s(-5, 3),       s(20, 17),      s(4, 22),       s(-11, 25),     s(24, 12),      s(18, 14),      s(17, 22),      s(10, 1),
            s(5, 34),       s(32, 24),      s(29, 33),      s(53, 16),      s(35, 24),      s(85, 26),      s(56, 20),      s(46, 24),
            s(-4, 27),      s(17, 37),      s(36, 33),      s(58, 48),      s(52, 36),      s(42, 37),      s(17, 31),      s(0, 26),
            s(1, 17),       s(1, 37),       s(14, 44),      s(51, 38),      s(45, 39),      s(12, 38),      s(7, 31),       s(23, 7),
            s(8, 22),       s(27, 33),      s(26, 35),      s(26, 38),      s(30, 43),      s(30, 34),      s(27, 24),      s(31, 13),
            s(24, 28),      s(26, 13),      s(36, 9),       s(14, 27),      s(29, 23),      s(41, 14),      s(48, 23),      s(31, 6),
            s(6, 14),       s(43, 23),      s(17, 19),      s(3, 20),       s(14, 19),      s(7, 33),       s(36, 5),       s(25, -5),
        };

        table[PieceType::ROOK] = {
            s(51, 113),     s(34, 123),     s(36, 133),     s(36, 128),     s(57, 117),     s(74, 112),     s(69, 112),     s(103, 100),
            s(30, 117),     s(30, 132),     s(54, 136),     s(80, 122),     s(65, 122),     s(103, 107),    s(94, 103),     s(131, 83),
            s(10, 121),     s(47, 121),     s(45, 123),     s(53, 119),     s(91, 102),     s(99, 95),      s(152, 85),     s(118, 78),
            s(2, 124),      s(21, 119),     s(21, 131),     s(31, 123),     s(42, 106),     s(45, 101),     s(66, 95),      s(61, 91),
            s(-13, 113),    s(-13, 118),    s(1, 119),      s(15, 116),     s(15, 112),     s(0, 111),      s(38, 94),      s(21, 89),
            s(-18, 107),    s(-8, 104),     s(4, 103),      s(7, 107),      s(18, 99),      s(20, 89),      s(61, 64),      s(28, 69),
            s(-19, 98),     s(-5, 100),     s(21, 99),      s(25, 101),     s(31, 89),      s(32, 83),      s(52, 72),      s(7, 81),
            s(5, 99),       s(15, 97),      s(32, 103),     s(42, 97),      s(49, 88),      s(37, 92),      s(47, 80),      s(8, 84),
        };

        table[PieceType::QUEEN] = {
            s(41, 135),     s(37, 153),     s(83, 165),     s(106, 163),    s(115, 159),    s(129, 141),    s(134, 106),    s(94, 126),
            s(73, 116),     s(40, 163),     s(53, 189),     s(56, 196),     s(70, 211),     s(108, 177),    s(85, 161),     s(154, 133),
            s(80, 119),     s(71, 140),     s(78, 168),     s(83, 197),     s(99, 200),     s(162, 179),    s(161, 141),    s(166, 132),
            s(60, 131),     s(63, 156),     s(64, 171),     s(66, 195),     s(71, 213),     s(91, 194),     s(95, 192),     s(104, 166),
            s(66, 123),     s(56, 161),     s(61, 168),     s(73, 192),     s(75, 183),     s(70, 182),     s(84, 165),     s(98, 150),
            s(67, 105),     s(75, 129),     s(72, 152),     s(71, 152),     s(78, 156),     s(87, 152),     s(101, 125),    s(99, 106),
            s(76, 97),      s(77, 101),     s(89, 102),     s(100, 103),    s(96, 110),     s(107, 79),     s(112, 48),     s(123, 24),
            s(74, 79),      s(78, 91),      s(90, 93),      s(99, 99),      s(96, 90),      s(75, 92),      s(88, 69),      s(68, 64),
        };

        table[PieceType::KING] = {
            s(-60, -108),   s(-6, -64),     s(2, -44),      s(-51, -5),     s(-59, -11),    s(-24, -1),     s(19, -4),      s(11, -101),
            s(-29, -28),    s(-31, 24),     s(-68, 36),     s(-2, 26),      s(-20, 45),     s(-15, 60),     s(2, 53),       s(-8, 11),
            s(-78, 0),      s(12, 34),      s(-59, 56),     s(-65, 67),     s(-37, 71),     s(32, 66),      s(16, 62),      s(-25, 20),
            s(-72, -8),     s(-87, 38),     s(-96, 62),     s(-138, 76),    s(-131, 77),    s(-101, 72),    s(-93, 59),     s(-121, 24),
            s(-78, -22),    s(-83, 19),     s(-125, 51),    s(-162, 71),    s(-166, 71),    s(-121, 54),    s(-121, 38),    s(-147, 15),
            s(-29, -33),    s(-6, -3),      s(-81, 25),     s(-102, 42),    s(-92, 41),     s(-89, 29),     s(-31, 4),      s(-53, -10),
            s(90, -59),     s(29, -24),     s(12, -9),      s(-31, 5),      s(-37, 10),     s(-14, -1),     s(51, -26),     s(63, -51),
            s(77, -104),    s(125, -86),    s(91, -57),     s(-43, -32),    s(38, -57),     s(-7, -35),     s(93, -72),     s(89, -112),
        };

        return table;
    }();
    // clang-format on

    constexpr std::array<std::array<I32, 28>, 4> k_MOBILITY_BONUS = []() {
        std::array<std::array<I32, 28>, 4> table{};

        // Knight (0-8)
        table[0] = {
          s(-50, -55), s(-22, -26), s(-7, 0),  s(2, 11), s(10, 21),
          s(18, 31),   s(27, 28),   s(36, 23), s(45, 8),
        };

        // Bishop (0-13)
        table[1] = {
          s(-43, -55), s(-25, -40), s(-9, -20), s(2, -2),  s(13, 11), s(20, 26), s(27, 31),
          s(31, 33),   s(33, 39),   s(39, 33),  s(48, 30), s(54, 28), s(43, 39), s(34, 19),
        };

        // Rook (0-14)
        table[2] = {
          s(-47, -40), s(-31, -16), s(-26, -12), s(-18, -8), s(-18, 2),
          s(-7, 7),    s(-1, 15),   s(9, 17),    s(20, 21),  s(28, 27),
          s(35, 28),   s(37, 34),   s(43, 39),   s(46, 36),  s(19, 45),
        };

        // Queen (0-27)
        table[3] = {
          s(-18, -10), s(-6, -37), s(-8, -33), s(0, -21), s(4, -7),  s(11, -2), s(17, 20),
          s(19, 39),   s(24, 48),  s(27, 54),  s(31, 62), s(36, 67), s(40, 69), s(41, 74),
          s(45, 75),   s(51, 76),  s(55, 79),  s(59, 79), s(67, 79), s(72, 79), s(75, 80),
          s(80, 80),   s(75, 78),  s(70, 76),  s(66, 72), s(52, 62), s(25, 35), s(17, 26),
        };

        return table;
    }();

    constexpr I32 k_BISHOP_PAIR = s(30, 84);
    constexpr I32 k_TEMPO_BONUS = s(15, 3);
}
