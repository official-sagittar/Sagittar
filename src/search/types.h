#pragma once

#include "core/defs.h"
#include "core/move.h"
#include "pch.h"

namespace sagittar {

    namespace search {

        struct SearchInfo {
            // Inputs
            bool infinite;
            int  wtime, btime, winc, binc, movetime, movestogo;
            int  depth;
            // Set by timeman
            bool     timeset;
            uint64_t starttime, stoptime;
        };

        struct SearchResult {
            core::Score score;
            bool        is_mate;
            int         mate_in;
            int         depth;
            int         nodes;
            int         time;
            core::Move  bestmove;
        };

    }

}
