#pragma once

#include "core/types.h"
#include "pch.h"

namespace sagittar::utils {

    template<typename T>
    inline constexpr T SEL(const bool cond, const T f, const T t) {
        return static_cast<T>((f) ^ (((f) ^ (t)) & -(cond)));
    }

    u64 prng();

    u64 currtimeInMilliseconds();

}
