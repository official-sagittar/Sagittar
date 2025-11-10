#pragma once

#include "commons/pch.h"
#include "core/position.h"
#include "core/types.h"

namespace sagittar::perft {

    size_t perft(const Position& pos, const Depth depth);
    size_t divide(const Position& pos, const Depth depth);

}
