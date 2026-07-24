#pragma once

#include "core/types.h"
#include "pch.h"

namespace sagittar {

    template<typename T>
    concept Indexable = std::integral<T> || std::is_enum_v<T>;

    template<Indexable T>
    [[nodiscard]] constexpr std::size_t index(T value) noexcept {
        if constexpr (std::is_enum_v<T>)
        {
            using Underlying = std::underlying_type_t<T>;
            return static_cast<std::size_t>(static_cast<Underlying>(value));
        }
        else
        {
            return static_cast<std::size_t>(value);
        }
    }

    namespace utils {

        u64 prng();

        u64 currtimeInMilliseconds();
    }

}
