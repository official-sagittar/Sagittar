#include "utils.h"

#include <chrono>
#include <cstdint>

namespace sagittar::utils {

    std::uint64_t prng() {
        // http://vigna.di.unimi.it/ftp/papers/xorshift.pdf

        static std::uint64_t seed = 1070372ULL;

        seed ^= seed >> 12;
        seed ^= seed << 25;
        seed ^= seed >> 27;

        return seed * 2685821657736338717ULL;
    }

    std::uint64_t currtimeInMilliseconds() {
        // Get the current time point
        auto now = std::chrono::system_clock::now();

        // Convert the time point to duration since epoch
        auto duration = now.time_since_epoch();

        // Convert duration to milliseconds
        auto milliseconds = std::chrono::duration_cast<std::chrono::milliseconds>(duration).count();

        return milliseconds;
    }

}  // namespace sagittar::utils
