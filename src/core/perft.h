#pragma once

#include "core/defs.h"
#include "core/position.h"
#include "core/tt.h"
#include "pch.h"

namespace sagittar {

    namespace core {

        uint64_t perft(const Position&                                          pos,
                       const int                                                depth,
                       TranspositionTable<TTClient::PERFT, uint64_t, uint32_t>& tt);
        uint64_t divide(const Position&                                          pos,
                        const int                                                depth,
                        TranspositionTable<TTClient::PERFT, uint64_t, uint32_t>& tt);

    }
}
