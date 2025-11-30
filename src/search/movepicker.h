#pragma once

#include "commons/containers.h"
#include "commons/pch.h"
#include "core/move.h"
#include "core/movegen.h"
#include "core/position.h"
#include "core/types.h"
#include "search/search.h"

namespace sagittar::search {

    enum class MovePickerPhase {
        TT_MOVE,
        CAPTURES,
        KILLERS,
        QUIETS,
        DONE
    };

    class MovePicker final {
       public:
        MovePicker() = delete;
        explicit MovePicker(ExtMove*                    buffer,
                            const Position&             pos,
                            const Move&                 ttmove,
                            const SearcherData&         data,
                            const Searcher::ThreadData& thread,
                            const i32                   ply,
                            const MovegenType           type);
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
        Move            next();

       private:
        void process(ExtMove*                    buffer,
                     const Position&             pos,
                     const Move&                 ttmove,
                     const SearcherData&         data,
                     const Searcher::ThreadData& thread,
                     const i32                   ply,
                     const MovegenType           type);

        size_t m_moves_count{0};

        std::span<ExtMove> m_captures;
        std::span<ExtMove> m_quiets;
        ExtMove*           m_it_caps;
        ExtMove*           m_it_quiets;

        Move                m_tt_move{};
        std::array<Move, 2> m_killers{};

        MovePickerPhase m_phase{MovePickerPhase::TT_MOVE};

        size_t m_index{0};
        u8     m_index_killers{0};
    };

}
