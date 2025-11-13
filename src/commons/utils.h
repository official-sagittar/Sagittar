#pragma once

#include "core/types.h"
#include "pch.h"

namespace sagittar::utils {

    template<typename T>
    inline constexpr T SEL(const bool cond, const T f, const T t) {
        return static_cast<T>((f) ^ (((f) ^ (t)) & -(cond)));
    }

    [[nodiscard]] u8 bitScanForward(u64* x);

    u8 bitCount1s(const u64 x);

    u64 prng();

    u64 currtimeInMilliseconds();

}
