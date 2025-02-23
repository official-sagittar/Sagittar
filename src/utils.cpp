#include "utils.h"

namespace sagittar {

    namespace utils {

        [[nodiscard]] u8 bitScanForward(u64* x) {
            u8 position = __builtin_ctzll(*x);
            *x &= *x - 1;
            return position;
        }

        u8 bitCount1s(const u64 x) { return __builtin_popcountll(x); }

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
            auto milliseconds =
              std::chrono::duration_cast<std::chrono::milliseconds>(duration).count();

            return milliseconds;
        }

        bool isFloat(const std::string& str) {
            static const std::regex floatRegex(R"([-+]?\d*\.\d+([eE][-+]?\d+)?)");
            return std::regex_match(str, floatRegex);
        }

    }

}
