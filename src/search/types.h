#pragma once

#include "commons/pch.h"
#include "core/move.h"
#include "core/types.h"

namespace sagittar::search {

#ifndef NDEBUG
    struct InstrumentationStore {
        size_t lmpEligible = 0;
        size_t lmpPruned   = 0;
    };
#endif

    struct SearchInfo {
        // Inputs
        bool  infinite;
        U32   wtime, btime, winc, binc, movetime, movestogo;
        Depth depth;
        // Set by timeman
        bool timeset;
        U64  starttime, stoptime;

        SearchInfo() :
            infinite(false),
            wtime(0),
            btime(0),
            winc(0),
            binc(0),
            movetime(0),
            movestogo(0),
            depth(0),
            timeset(false),
            starttime(0ULL),
            stoptime(0ULL) { }
    };

    struct SearchResult {
        Score             score;
        bool              is_mate;
        I8                mate_in;
        Depth             depth;
        size_t            nodes;
        U64               time;
        U32               hashfull;
        std::vector<Move> pv;
        Move              bestmove{};
#ifndef NDEBUG
        InstrumentationStore instrumentation{};
#endif
    };

}
