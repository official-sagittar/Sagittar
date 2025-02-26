#include "timeman.h"
#include "search.h"
#include "types.h"
#include "utils.h"

namespace sagittar {

    namespace search {

        namespace timeman {

            void setSearchHardBoundTime(SearchInfo* info, const board::Board& board) {
                if (info->infinite)
                {
                    info->timeset = false;
                    info->depth   = search::MAX_DEPTH;
                    return;
                }

                if (info->movetime > 0)
                {
                    info->timeset   = true;
                    info->starttime = utils::currtimeInMilliseconds();
                    info->stoptime  = info->starttime + info->movetime;
                    return;
                }

                if (info->depth > 0)
                {
                    info->timeset = false;
                    return;
                }

                u32 time, inc;

                if (board.getActiveColor() == Color::WHITE)
                {
                    time = info->wtime;
                    inc  = info->winc;
                }
                else
                {
                    time = info->btime;
                    inc  = info->binc;
                }

                if (info->movestogo == 0)
                {
                    info->movestogo = 30;
                }

                info->depth     = search::MAX_DEPTH;
                info->timeset   = true;
                info->starttime = utils::currtimeInMilliseconds();
                time /= info->movestogo;
                time -= 50;
                info->stoptime = info->starttime + time + inc;
            }

        }

    }

}
