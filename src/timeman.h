#pragma once

#include "board.h"
#include "pch.h"
#include "searchtypes.h"

namespace sagittar {

    namespace search {

        namespace timeman {

            void setSearchHardBoundTime(SearchInfo* info, const board::Board& board);

        }

    }

}
