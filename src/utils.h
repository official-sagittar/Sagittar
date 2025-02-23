#pragma once

#include "pch.h"
#include "types.h"

namespace sagittar {

    namespace utils {

        [[nodiscard]] u8 bitScanForward(u64* x);

        u8 bitCount1s(const u64 x);

        u64 prng();

        u64 currtimeInMilliseconds();

        bool isFloat(const std::string& str);

    }

}
