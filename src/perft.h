#pragma once

#include "board.h"
#include "pch.h"
#include "types.h"

namespace sagittar {

    namespace perft {

        u64 perft(const board::Board& board, const Depth depth);
        u64 divide(const board::Board& board, const Depth depth);

    }

}
