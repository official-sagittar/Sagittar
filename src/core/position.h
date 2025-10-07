#pragma once

#include "core/defs.h"
#include "core/move.h"
#include "pch.h"

namespace sagittar {

    namespace core {

        class Position {
           public:
            Position()                           = default;
            Position(const Position&)            = default;
            Position& operator=(const Position&) = default;
            ~Position()                          = default;

            void reset();
            void reset_key();

            bool set_fen(std::string);

            Piece    piece_on(const Square sq) const;
            BitBoard pieces(const Color c) const;
            template<typename... PieceType>
            BitBoard pieces(PieceType... pts) const;
            template<typename... PieceType>
            BitBoard pieces(const Color c, PieceType... pts) const;
            Square   king_sq() const;
            BitBoard checkers() const;
            BitBoard pinned() const;
            Color    stm() const;
            uint8_t  ca_rights() const;
            Square   ep_target() const;
            Key      key() const;

            bool is_valid() const;
            bool is_drawn(std::span<uint64_t> key_history) const;
            bool is_in_check() const;

            bool                      is_legal_move(const Move move) const;
            Position                  do_move(const Move move) const;
            std::pair<bool, Position> do_move(const std::string& move_str) const;

            void display() const;

           private:
            void set_piece(const PieceType pt, const Color c, const Square sq);
#ifdef DEBUG
            void assert_valid() const;
#endif

            // Board
            std::array<BitBoard, 7> m_bb_pieces{};
            std::array<BitBoard, 2> m_bb_colors{};
            std::array<Piece, 64>   m_board{};
            // Status
            Square   m_king_sq;
            BitBoard m_checkers;
            BitBoard m_pinned;
            // Position context
            bool     m_black_to_play;
            uint8_t  m_ca_rights;
            Square   m_ep_target;
            uint32_t m_half_moves, m_full_moves, m_ply_count;
            // Key
            Key m_key;
        };

        template<typename... PieceType>
        inline BitBoard Position::pieces(PieceType... pts) const {
            return (m_bb_pieces[pts] | ...);
        }

        template<typename... PieceType>
        inline BitBoard Position::pieces(const Color c, PieceType... pts) const {
            return pieces(c) & pieces(pts...);
        }

    }

}
