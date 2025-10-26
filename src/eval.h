#pragma once

#include "pch.h"
#include "position.h"
#include "types.h"

namespace sagittar {

    namespace eval {

        void  initialize();
        Score evaluate(const core::Position&);
        bool  isEndGame(const core::Position&);

    }

}
