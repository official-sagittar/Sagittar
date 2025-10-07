#pragma once

#include "core/defs.h"
#include "pch.h"

namespace sagittar {

    namespace core {

        template<PieceType PT>
        BitBoard attacks(const Square sq, const BitBoard occupancy, const Color c = WHITE);

    }

}
