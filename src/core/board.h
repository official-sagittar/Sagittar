#pragma once

#include "core/defs.h"
#include "pch.h"

namespace sagittar {

    namespace core {

        class Board {
           public:
            Board();
            Board(const Board&) = default;
            void   reset();
            void   set_piece(const PieceType pt, const Color c, const Square sq);
            bool   is_valid() const;
            void   display() const;
            Board& operator=(const Board&) = default;
#ifdef DEBUG
            void assert_valid() const;
#endif
            ~Board() = default;

           public:
            std::array<BitBoard, 7> bb_pieces;
            std::array<BitBoard, 2> bb_colors;
            std::array<Piece, 64>   pieces;
        };

    }

}
