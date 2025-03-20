#pragma once

#include "commons/pch.h"
#include "commons/types.h"

namespace sagittar {

    namespace utils {

        using namespace commons::types;

        [[nodiscard]] u8 bitScanForward(u64* x);

        u8 bitCount1s(const u64 x);

        u64 prng();

        u64 currtimeInMilliseconds();

    }

}
