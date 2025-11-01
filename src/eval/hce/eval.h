#pragma once

#include "commons/pch.h"
#include "core/position.h"
#include "core/types.h"

namespace sagittar::eval::hce {

    void  initialize();
    Score evaluate(const Position&);
    bool  isEndGame(const Position&);

}
