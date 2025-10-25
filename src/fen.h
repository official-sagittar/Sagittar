#pragma once

#include "board.h"
#include "pch.h"

namespace sagittar {

    namespace fen {

        void        parseFEN(board::Board*, const std::string&, const bool full = true);
        std::string toFEN(const board::Board&);

    }

}
