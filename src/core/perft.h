#pragma once

#include "core/defs.h"
#include "core/position.h"
#include "core/tt.h"
#include "pch.h"

namespace sagittar {

    namespace core {

        uint64_t perft(Position* const pos, const int depth, TranspositionTable* const tt);
        uint64_t divide(Position* const pos, const int depth, TranspositionTable* const tt);

    }
}
