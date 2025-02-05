#pragma once

#include "board.h"
#include "pch.h"
#include "types.h"

namespace sagittar {

    namespace eval {

        void initialize();
        i32  evaluateBoard(const board::Board&);
        bool isEndGame(const board::Board&);

    }

}
