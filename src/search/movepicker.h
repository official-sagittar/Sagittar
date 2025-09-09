#pragma once

#include "core/defs.h"
#include "core/move.h"
#include "core/position.h"
#include "pch.h"

namespace sagittar {

    namespace search {

        using namespace sagittar::core;

        class MovePicker {
           public:
            MovePicker(MoveList* const moves_list, const Position* const pos, const Move pv_move);
            bool                   has_next() const;
            std::pair<Move, Score> next();

           private:
            size_t          index;
            MoveList* const list;
        };

    }

}
