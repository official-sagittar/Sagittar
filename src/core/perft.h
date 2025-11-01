#pragma once

#include "commons/pch.h"
#include "core/position.h"
#include "core/types.h"

namespace sagittar::perft {

    u64 perft(const Position& pos, const Depth depth);
    u64 divide(const Position& pos, const Depth depth);

}
