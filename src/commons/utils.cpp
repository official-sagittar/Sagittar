#include "utils.h"

namespace sagittar::utils {

    u64 prng() {
        // http://vigna.di.unimi.it/ftp/papers/xorshift.pdf

        static u64 seed = 1070372ULL;

        seed ^= seed >> 12;
        seed ^= seed << 25;
        seed ^= seed >> 27;

        return seed * 2685821657736338717ULL;
    }

    u64 currtimeInMilliseconds() {
        // Get the current time point
        auto now = std::chrono::system_clock::now();

        // Convert the time point to duration since epoch
        auto duration = now.time_since_epoch();

        // Convert duration to milliseconds
        auto milliseconds = std::chrono::duration_cast<std::chrono::milliseconds>(duration).count();

        return milliseconds;
    }

}
