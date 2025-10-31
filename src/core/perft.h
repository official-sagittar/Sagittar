#pragma once

#include "commons/pch.h"
#include "core/position.h"
#include "core/types.h"

namespace sagittar {

    namespace perft {

        u64 perft(const core::Position& pos, const Depth depth);
        u64 divide(const core::Position& pos, const Depth depth);

    }

}
