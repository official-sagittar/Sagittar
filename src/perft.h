#pragma once

#include "board.h"
#include "pch.h"
#include "types.h"

namespace sagittar {

    namespace perft {

        u64 perft(board::Board& board, const Depth depth);
        u64 divide(board::Board& board, const Depth depth);

    }

}
