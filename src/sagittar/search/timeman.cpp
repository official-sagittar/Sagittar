#include "sagittar/search/timeman.h"
#include "commons/types.h"
#include "sagittar/core/types.h"
#include "sagittar/search/search.h"
#include "sagittar/utils/utils.h"

namespace sagittar {

    namespace search {

        namespace timeman {

            using namespace commons::types;
            using namespace core::types;

            void setSearchHardBoundTime(SearchInfo* info, const core::board::Board& board) {
                u32 time = 0, inc = 0;

                if (info->movetime > 0)
                {
                    time            = info->movetime;
                    info->movestogo = 1;
                }
                else if (!info->infinite)
                {
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
                }

                info->depth = info->depth == 0 ? search::MAX_DEPTH : info->depth;

                info->timeset = false;

                if (time > 0)
                {
                    info->movestogo = info->movestogo == 0 ? 30 : info->movestogo;
                    info->timeset   = true;
                    info->starttime = utils::currtimeInMilliseconds();
                    time /= info->movestogo;
                    time -= 50;
                    info->stoptime = info->starttime + time + inc;
                }
            }

        }

    }

}
