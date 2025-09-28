#pragma once

#include "core/defs.h"
#include "core/move.h"
#include "core/position.h"
#include "pch.h"
#include "search/history.h"

namespace sagittar {

    namespace search {

        using namespace sagittar::core;

        class MovePicker {
           public:
            MovePicker(MoveList* const       moves_list,
                       const Position&       pos,
                       const Move            tt_move,
                       const PieceToHistory& history);
            bool                   has_next() const;
            std::pair<Move, Score> next();

           private:
            size_t          index;
            MoveList* const list;
        };

    }

}
