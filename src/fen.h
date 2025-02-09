#pragma once

#include "board.h"
#include "pch.h"

namespace sagittar {

    namespace fen {

        void        parseFEN(board::Board*, std::string, const bool full = true);
        std::string toFEN(const board::Board&);

    }

}
