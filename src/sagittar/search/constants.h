#pragma once

#include "sagittar/core/types.h"

namespace sagittar {

    namespace search {

        namespace constants {

            using namespace core::types;

            constexpr Score INF        = 15000;
            constexpr Score MATE_VALUE = 14000;
            constexpr Score MATE_SCORE = 13000;
            constexpr Score WIN_SCORE  = 12000;
            constexpr Depth MAX_DEPTH  = 64;

            constexpr std::size_t DEFAULT_TT_SIZE_MB = 16;

        }

    }

}
