#pragma once

#include "pch.h"
#include "position.h"
#include "types.h"

namespace sagittar {

    namespace perft {

        u64 perft(const core::Position& pos, const Depth depth);
        u64 divide(const core::Position& pos, const Depth depth);

    }

}
