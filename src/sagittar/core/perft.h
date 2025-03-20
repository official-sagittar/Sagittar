#pragma once

#include "commons/pch.h"
#include "commons/types.h"
#include "sagittar/core/board.h"
#include "sagittar/core/types.h"

namespace sagittar {

    namespace core {

        namespace perft {

            using namespace commons::types;
            using namespace core::types;

            u64 perft(board::Board& board, const Depth depth);
            u64 divide(board::Board& board, const Depth depth);

        }

    }

}
