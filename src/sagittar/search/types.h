#pragma once

#include "commons/pch.h"
#include "commons/types.h"
#include "sagittar/core/move.h"
#include "sagittar/core/types.h"

namespace sagittar {

    namespace search {

        namespace types {

            using namespace commons::types;
            using namespace core::types;

            struct SearchInfo {
                // Inputs
                bool  infinite;
                u32   wtime, btime, winc, binc, movetime, movestogo;
                Depth depth;
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
                Score                         score;
                bool                          is_mate;
                i8                            mate_in;
                Depth                         depth;
                u64                           nodes;
                u64                           time;
                u32                           hashfull;
                std::vector<core::move::Move> pv;
                core::move::Move              bestmove{};
            };

        }

    }

}
