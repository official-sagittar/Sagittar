#pragma once

#include "commons/pch.h"
#include "sagittar/core/board.h"
#include "sagittar/search/types.h"

namespace sagittar {

    namespace search {

        namespace timeman {

            using namespace search::types;

            void setSearchHardBoundTime(SearchInfo* info, const core::board::Board& board);

        }

    }

}
