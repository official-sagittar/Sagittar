#pragma once

#include "board.h"
#include "containers.h"
#include "move.h"
#include "movegen.h"
#include "pch.h"
#include "search.h"
#include "types.h"

namespace sagittar {

    namespace search {

        enum class MovePickerPhase {
            TT_MOVE,
            CAPTURES,
            KILLERS,
            QUIETS,
            DONE
        };

        template<movegen::MovegenType T>
        class MovePicker {
           public:
            MovePicker() = delete;
            explicit MovePicker(move::ExtMove*      ext_moves,
                                const board::Board& board,
                                const move::Move&   ttmove,
                                const SearcherData& data,
                                const i32           ply);
            MovePicker(const MovePicker&)                = delete;
            MovePicker(MovePicker&&) noexcept            = delete;
            MovePicker& operator=(const MovePicker&)     = delete;
            MovePicker& operator=(MovePicker&&) noexcept = delete;
            void*       operator new(std::size_t)        = delete;
            void*       operator new[](std::size_t)      = delete;
            ~MovePicker() noexcept                       = default;

            size_t          size() const;
            MovePickerPhase phase() const;
            bool            hasNext() const;
            move::Move      next();

           private:
            void processMoves(const containers::ArrayList<move::Move>& moves,
                              const board::Board&                      board,
                              const move::Move&                        ttmove,
                              const SearcherData&                      data,
                              const i32                                ply);

            move::ExtMove*            m_ext_moves_begin;
            move::ExtMove*            m_ext_moves_end;
            move::ExtMove*            m_ext_moves_it;
            move::Move                m_tt_move{};
            std::array<move::Move, 2> m_killers{};

            MovePickerPhase m_phase{MovePickerPhase::TT_MOVE};

            size_t m_moves_count{0};
            size_t m_capture_moves_count{0};

            size_t m_index{0};
            size_t m_index_captures{0};
            size_t m_index_killers{0};
        };

    }

}
