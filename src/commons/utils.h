#pragma once

#include "core/types.h"
#include "pch.h"

namespace sagittar::utils {

    [[nodiscard]] u8 bitScanForward(u64* x);

    u8 bitCount1s(const u64 x);

    u64 prng();

    u64 currtimeInMilliseconds();

}
