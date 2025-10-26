#pragma once

#include "pch.h"
#include "position.h"

namespace sagittar {

    namespace fen {

        void        parseFEN(core::Position*, std::string, const bool full = true);
        std::string toFEN(const core::Position&);

    }

}
