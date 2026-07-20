#pragma once

#include "commons/pch.h"
#include "core/types.h"

namespace sagittar::search {

    static constexpr i16 MAX_HISTORY = std::numeric_limits<i16>::max();

    using PieceToHistory = std::array<std::array<i16, 64>, 15>;

}
