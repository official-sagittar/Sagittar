#pragma once

#include "commons/containers.h"
#include "commons/pch.h"
#include "core/move.h"
#include "core/types.h"

namespace sagittar {

    using BitBoard = u64;

    constexpr BitBoard BB(const Square sq) { return static_cast<BitBoard>(1ULL << sq); }
    constexpr BitBoard BB(const int sq) { return static_cast<BitBoard>(1ULL << sq); }

    constexpr BitBoard MASK64(const bool x) { return -static_cast<BitBoard>(x); }

    constexpr BitBoard MASK_RANK_1       = 0xFFULL;
    constexpr BitBoard MASK_RANK_2       = MASK_RANK_1 << (8 * 1);
    constexpr BitBoard MASK_RANK_3       = MASK_RANK_1 << (8 * 2);
    constexpr BitBoard MASK_RANK_4       = MASK_RANK_1 << (8 * 3);
    constexpr BitBoard MASK_RANK_5       = MASK_RANK_1 << (8 * 4);
    constexpr BitBoard MASK_RANK_6       = MASK_RANK_1 << (8 * 5);
    constexpr BitBoard MASK_RANK_7       = MASK_RANK_1 << (8 * 6);
    constexpr BitBoard MASK_RANK_8       = MASK_RANK_1 << (8 * 7);
    constexpr BitBoard MASK_RANK_1_AND_8 = MASK_RANK_1 & MASK_RANK_8;

    constexpr BitBoard MASK_FILE_A      = 0x0101010101010101ULL;
    constexpr BitBoard MASK_FILE_B      = MASK_FILE_A << 1;
    constexpr BitBoard MASK_FILE_C      = MASK_FILE_A << 2;
    constexpr BitBoard MASK_FILE_D      = MASK_FILE_A << 3;
    constexpr BitBoard MASK_FILE_E      = MASK_FILE_A << 4;
    constexpr BitBoard MASK_FILE_F      = MASK_FILE_A << 5;
    constexpr BitBoard MASK_FILE_G      = MASK_FILE_A << 6;
    constexpr BitBoard MASK_FILE_H      = MASK_FILE_A << 7;
    constexpr BitBoard MASK_NOT_A_FILE  = ~MASK_FILE_A;
    constexpr BitBoard MASK_NOT_H_FILE  = ~MASK_FILE_H;
    constexpr BitBoard MASK_NOT_AB_FILE = ~(MASK_FILE_A | MASK_FILE_B);
    constexpr BitBoard MASK_NOT_GH_FILE = ~(MASK_FILE_G | MASK_FILE_H);

    constexpr BitBoard MASK_RANK(const Rank r) { return MASK_RANK_1 << (8 * r); }
    constexpr BitBoard MASK_FILE(const File f) { return MASK_FILE_A << f; }

    constexpr BitBoard MASK_WKCA_PATH = 0x60;
    constexpr BitBoard MASK_WQCA_PATH = 0xE;
    constexpr BitBoard MASK_BKCA_PATH = 0x6000000000000000;
    constexpr BitBoard MASK_BQCA_PATH = 0xE00000000000000;

    constexpr BitBoard north(const BitBoard b) { return b << 8; }
    constexpr BitBoard south(const BitBoard b) { return b >> 8; }
    constexpr BitBoard east(const BitBoard b) { return (b & MASK_NOT_H_FILE) << 1; }
    constexpr BitBoard west(const BitBoard b) { return (b & MASK_NOT_A_FILE) >> 1; }
    constexpr BitBoard northEast(const BitBoard b) { return (b & MASK_NOT_H_FILE) << 9; }
    constexpr BitBoard southEast(const BitBoard b) { return (b & MASK_NOT_H_FILE) >> 7; }
    constexpr BitBoard southWest(const BitBoard b) { return (b & MASK_NOT_A_FILE) >> 9; }
    constexpr BitBoard northWest(const BitBoard b) { return (b & MASK_NOT_A_FILE) << 7; }

    enum CastleFlag : u8 {
        NOCA = 0,
        WKCA = 1,
        WQCA = 2,
        BKCA = 4,
        BQCA = 8
    };

    class Position {
       public:
        static void initialize();

        Position();
        Position(const Position&)                = default;
        Position(Position&&) noexcept            = default;
        Position& operator=(const Position&)     = default;
        Position& operator=(Position&&) noexcept = default;
        ~Position()                              = default;

        void reset();

        void        setFen(std::string, const bool full = true);
        std::string toFen() const;

        [[nodiscard]] bool doMove(const Move&) noexcept;
        [[nodiscard]] bool doMove(const std::string&) noexcept;
        void               doNullMove();

        BitBoard pieces(const Color) const;
        BitBoard pieces(const PieceType) const;
        BitBoard pieces(const Color, const PieceType) const;
        BitBoard occupied() const;
        BitBoard empty() const;
        Piece    pieceOn(const Square) const;
        u8       pieceCount(const Piece) const;

        Color  stm() const;
        u8     caRights() const;
        Square epTarget() const;
        u8     halfmoves() const;
        u8     fullmoves() const;
        u64    key() const;

        bool isValid() const;
        bool isInCheck() const;
        bool isDrawn(std::span<u64> key_history) const;

        void display() const;

        bool operator==(Position const& rhs) const;

       private:
        void resetHash();

        template<Color US, MoveFlag F>
        bool applyMove(const Move& move) noexcept;

        template<Color US>
        using ApplyMoveFn = bool (Position::*)(const Move&);

        template<Color US>
        static constexpr std::array<ApplyMoveFn<US>, 16> apply_move_dispatch_table = []() {
            std::array<ApplyMoveFn<US>, 16> table{};

            table[MOVE_QUIET]               = &Position::applyMove<US, MOVE_QUIET>;
            table[MOVE_QUIET_PAWN_DBL_PUSH] = &Position::applyMove<US, MOVE_QUIET_PAWN_DBL_PUSH>;
            table[MOVE_CASTLE_KING_SIDE]    = &Position::applyMove<US, MOVE_CASTLE_KING_SIDE>;
            table[MOVE_CASTLE_QUEEN_SIDE]   = &Position::applyMove<US, MOVE_CASTLE_QUEEN_SIDE>;
            table[MOVE_CAPTURE]             = &Position::applyMove<US, MOVE_CAPTURE>;
            table[MOVE_CAPTURE_EP]          = &Position::applyMove<US, MOVE_CAPTURE_EP>;
            table[MOVE_PROMOTION_KNIGHT]    = &Position::applyMove<US, MOVE_PROMOTION_KNIGHT>;
            table[MOVE_PROMOTION_BISHOP]    = &Position::applyMove<US, MOVE_PROMOTION_BISHOP>;
            table[MOVE_PROMOTION_ROOK]      = &Position::applyMove<US, MOVE_PROMOTION_ROOK>;
            table[MOVE_PROMOTION_QUEEN]     = &Position::applyMove<US, MOVE_PROMOTION_QUEEN>;
            table[MOVE_CAPTURE_PROMOTION_KNIGHT] =
              &Position::applyMove<US, MOVE_CAPTURE_PROMOTION_KNIGHT>;
            table[MOVE_CAPTURE_PROMOTION_BISHOP] =
              &Position::applyMove<US, MOVE_CAPTURE_PROMOTION_BISHOP>;
            table[MOVE_CAPTURE_PROMOTION_ROOK] =
              &Position::applyMove<US, MOVE_CAPTURE_PROMOTION_ROOK>;
            table[MOVE_CAPTURE_PROMOTION_QUEEN] =
              &Position::applyMove<US, MOVE_CAPTURE_PROMOTION_QUEEN>;

            return table;
        }();

        std::array<BitBoard, 7> m_bb_pieces;
        std::array<BitBoard, 2> m_bb_colors;
        std::array<Piece, 64>   m_board;
        BitBoard                m_checkers;
        Color                   m_stm;
        u8                      m_ca_rights;
        Square                  m_ep_target;
        u8                      m_halfmoves;
        u8                      m_fullmoves;
        i32                     m_ply_count;
        u64                     m_key;
    };

}
