#pragma once

#include "core/position.h"
#include "core/types.h"

namespace sagittar::eval::hce {

    Score evaluate(const Position&);
    bool  isEndGame(const Position&);

}  // namespace sagittar::eval::hce
