#pragma once

#include "commons/pch.h"
#include "core/position.h"
#include "search/searchtypes.h"

namespace sagittar {

    namespace search {

        namespace timeman {

            void setSearchHardBoundTime(SearchInfo* info, const core::Position& pos);

        }

    }

}
