#pragma once

#include "containers.h"
#include "move.h"
#include "pch.h"
#include "types.h"

namespace sagittar {

    namespace board {

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

        constexpr u8 bitboardColorSlot(const Piece p) { return (7 + pieceColorOf(p)); }
        constexpr u8 bitboardColorSlot(const Color c) { return (7 + c); }

        enum CastleFlag : u8 {
            NOCA = 0,
            WKCA = 1,
            WQCA = 2,
            BKCA = 4,
            BQCA = 8
        };

        enum class DoMoveResult : u8 {
            INVALID,  // Incorrect Move
            ILLEGAL,  // Move is correct, but results in check or invalid board
            LEGAL     // Move is correct and move is legal
        };

        class Board {
           private:
            std::array<BitBoard, 15> bitboards;
            std::array<Piece, 64>    pieces;
            BitBoard                 checkers;
            Color                    active_color;
            u8                       casteling_rights;
            Square                   enpassant_target;
            u8                       half_move_clock;
            u8                       full_move_number;
            i32                      ply_count;
            u64                      hash;

            DoMoveResult doMoveComplete();

           public:
            static void initialize();

            Board();
            ~Board();
            Board(const Board&)            = default;
            Board& operator=(const Board&) = default;
            bool   operator==(Board const& rhs) const;

            void                       reset();
            void                       resetHash();
            void                       setPiece(const Piece, const Square);
            void                       clearPiece(const Piece, const Square);
            void                       setStartpos();
            void                       movePiece(const Piece  piece,
                                                 const Square from,
                                                 const Square to,
                                                 const bool   is_capture   = false,
                                                 const bool   is_promotion = false,
                                                 const Piece  promoted     = Piece::NO_PIECE);
            void                       undoMovePiece(const Piece  piece,
                                                     const Square from,
                                                     const Square to,
                                                     const bool   is_capture   = false,
                                                     const Piece  captured     = Piece::NO_PIECE,
                                                     const bool   is_promotion = false,
                                                     const Piece  promoted     = Piece::NO_PIECE);
            void                       setCheckers(const BitBoard bb);
            void                       setActiveColor(const Color);
            void                       addCastelingRights(const CastleFlag);
            void                       setEnpassantTarget(const Square);
            void                       setHalfmoveClock(const u8);
            void                       setFullmoveNumber(const u8);
            [[nodiscard]] DoMoveResult doMove(const move::Move&) noexcept;
            [[nodiscard]] DoMoveResult doMove(const std::string&) noexcept;
            void                       doNullMove();
            BitBoard                   getBitboard(const u8 index) const;
            BitBoard                   getBitboard(const PieceType, const Color) const;
            Piece                      getPiece(const Square) const;
            u8                         getPieceCount(const Piece) const;
            Color                      getActiveColor() const;
            u8                         getCastelingRights() const;
            Square                     getEnpassantTarget() const;
            u8                         getHalfmoveClock() const;
            u8                         getFullmoveNumber() const;
            u64                        getHash() const;
            bool                       isValid() const;
            bool                       isInCheck() const;
            bool                       hasPositionRepeated(std::span<u64> key_history) const;
            void                       display() const;
        };

    }

}
