#pragma once

#include "core/defs.h"
#include "pch.h"

namespace sagittar {

    namespace core {

        template<typename T>
        constexpr T SEL(const bool cond, const T f, const T t) {
            return static_cast<T>((f) ^ (((f) ^ (t)) & -(cond)));
        }
        constexpr BitBoard MASK64(const bool x) { return -static_cast<BitBoard>(x); }

        uint64_t prng();

        int bitscan_forward(uint64_t* const x);

        void* aligned_alloc(const size_t alignment, const size_t count, const size_t size);

        void aligned_free(void* ptr);

    }

}
