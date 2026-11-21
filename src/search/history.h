#pragma once

#include "commons/pch.h"
#include "core/types.h"

namespace sagittar::search {

    static constexpr I16 k_MAX_HISTORY = std::numeric_limits<I16>::max();

    using PieceToHistory = std::array<std::array<I16, 64>, 15>;

}
