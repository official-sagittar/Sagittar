#pragma once

#include "commons/containers.h"
#include "commons/pch.h"
#include "core/move.h"
#include "core/types.h"

namespace sagittar {

    using BitBoard = u64;

    constexpr BitBoard MASK_RANK_1      = 0xFF;
    constexpr BitBoard MASK_RANK_2      = MASK_RANK_1 << (8 * 1);
    constexpr BitBoard MASK_RANK_3      = MASK_RANK_1 << (8 * 2);
    constexpr BitBoard MASK_RANK_4      = MASK_RANK_1 << (8 * 3);
    constexpr BitBoard MASK_RANK_5      = MASK_RANK_1 << (8 * 4);
    constexpr BitBoard MASK_RANK_6      = MASK_RANK_1 << (8 * 5);
    constexpr BitBoard MASK_RANK_7      = MASK_RANK_1 << (8 * 6);
    constexpr BitBoard MASK_RANK_8      = MASK_RANK_1 << (8 * 7);
    constexpr BitBoard MASK_NOT_A_FILE  = 0xFEFEFEFEFEFEFEFE;
    constexpr BitBoard MASK_NOT_H_FILE  = 0x7F7F7F7F7F7F7F7F;
    constexpr BitBoard MASK_NOT_AB_FILE = 0xFCFCFCFCFCFCFCFC;
    constexpr BitBoard MASK_NOT_GH_FILE = 0x3F3F3F3F3F3F3F3F;

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

    enum class DoMoveResult : u8 {
        INVALID,  // Incorrect Move
        ILLEGAL,  // Move is correct, but results in check or invalid pos
        LEGAL     // Move is correct and move is legal
    };

    class Position {
       private:
        void         resetHash();
        void         setPiece(const Piece, const Square);
        void         clearPiece(const Piece, const Square);
        void         movePiece(const Piece  piece,
                               const Square from,
                               const Square to,
                               const bool   is_capture   = false,
                               const bool   is_promotion = false,
                               const Piece  promoted     = Piece::NO_PIECE);
        DoMoveResult doMoveComplete();

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

        [[nodiscard]] DoMoveResult doMove(const Move&) noexcept;
        [[nodiscard]] DoMoveResult doMove(const std::string&) noexcept;
        void                       doNullMove();

        BitBoard pieces(const Color) const;
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
        std::array<BitBoard, 15> m_bitboards;
        std::array<Piece, 64>    m_board;
        BitBoard                 m_checkers;
        Color                    m_stm;
        u8                       m_ca_rights;
        Square                   m_ep_target;
        u8                       m_halfmoves;
        u8                       m_fullmoves;
        i32                      m_ply_count;
        u64                      m_key;
    };

}
