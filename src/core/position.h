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
        template<typename... PieceTypes>
        inline BitBoard pieces(PieceTypes... pts) const {
            return (m_bb_pieces[pts] | ...);
        }
        BitBoard pieces(const Color, const PieceType) const;
        template<typename... PieceTypes>
        inline BitBoard pieces(const Color c, PieceTypes... pts) const {
            return m_bb_colors[c] & pieces(pts...);
        }
        BitBoard occupied() const;
        BitBoard empty() const;
        Piece    pieceOn(const Square) const;
        U8       pieceCount(const PieceType) const;
        U8       pieceCount(const Piece) const;

        inline BitBoard checkers() const { return m_checkers; }
        inline Square   kingSq() const { return m_king_sq; }

        Color  stm() const;
        U8     caRights() const;
        Square epTarget() const;
        U8     halfmoves() const;
        U8     fullmoves() const;
        U64    key() const;
        U64    pawnKey() const;

        bool isValid() const;
        bool isInCheck() const;
        bool isDrawn(std::span<U64> key_history) const;

        void display() const;

        bool operator==(Position const& rhs) const;

       private:
        void resetHash();

        template<Color US, MoveFlag F>
        bool applyMove(const Move& move) noexcept;

        template<Color US>
        using ApplyMoveFn = bool (Position::*)(const Move&);

        template<Color US>
        static constexpr std::array<ApplyMoveFn<US>, 16> k_APPLY_MOVE_DISPATCH_TABLE = []() {
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

        std::array<BitBoard, 6> m_bb_pieces;
        std::array<BitBoard, 2> m_bb_colors;
        std::array<Piece, 64>   m_board;
        BitBoard                m_checkers;
        Square                  m_king_sq;
        Color                   m_stm;
        U8                      m_ca_rights;
        Square                  m_ep_target;
        U8                      m_halfmoves;
        U8                      m_fullmoves;
        I32                     m_ply_count;
        U64                     m_key;
        U64                     m_pawn_key;
    };

}
