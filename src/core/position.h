#pragma once

#include "commons/containers.h"
#include "commons/pch.h"
#include "core/bitboard.h"
#include "core/move.h"
#include "core/types.h"

namespace sagittar {

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
