#include "timeman.h"
#include "core/utils.h"

namespace sagittar {

    namespace search {

        template<Color US>
        void set_hardbound_time(SearchInfo* info) {
            uint32_t time = 0, inc = 0;

            if (info->movetime > 0)
            {
                time            = info->movetime;
                info->movestogo = 1;
            }
            else if (!info->infinite)
            {
                if constexpr (US == WHITE)
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

            info->depth = (info->depth == 0) ? (DEPTH_MAX - 1) : info->depth;

            info->timeset = false;

            if (time > 0)
            {
                info->movestogo = info->movestogo == 0 ? 30 : info->movestogo;
                info->timeset   = true;
                info->starttime = currtime_ms();
                time /= info->movestogo;
                time -= 50;
                info->stoptime = info->starttime + time + inc;
            }
        }

        template void set_hardbound_time<WHITE>(SearchInfo* info);
        template void set_hardbound_time<BLACK>(SearchInfo* info);

    }

}
