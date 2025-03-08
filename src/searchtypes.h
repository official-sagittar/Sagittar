#pragma once

#include "move.h"
#include "pch.h"
#include "types.h"

namespace sagittar {

    namespace search {

        struct SearchInfo {
            // Inputs
            bool infinite;
            u32  wtime, btime, winc, binc, movetime, movestogo;
            u8   depth;
            // Set by timeman
            bool timeset;
            u64  starttime, stoptime;

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
                stoptime(0ULL) {}
        };

        struct SearchResult {
            i32                     score;
            bool                    is_mate;
            i8                      mate_in;
            u8                      depth;
            u64                     nodes;
            u64                     time;
            u32                     hashfull;
            std::vector<move::Move> pv;
            move::Move              bestmove{};
        };

    }
}
