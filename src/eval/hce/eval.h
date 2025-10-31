#pragma once

#include "commons/pch.h"
#include "core/position.h"
#include "core/types.h"

namespace sagittar {

    namespace eval {

        void  initialize();
        Score evaluate(const core::Position&);
        bool  isEndGame(const core::Position&);

    }

}
