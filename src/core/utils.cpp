#include "utils.h"

namespace sagittar {

    namespace core {

        uint64_t prng() {
            // http://vigna.di.unimi.it/ftp/papers/xorshift.pdf

            static uint64_t seed = 1070372ULL;

            seed ^= seed >> 12;
            seed ^= seed << 25;
            seed ^= seed >> 27;

            return seed * 2685821657736338717ULL;
        }

        int bitscan_forward(uint64_t* const x) {
            const int position = __builtin_ctzll(*x);
            *x &= *x - 1;
            return position;
        }

        void* aligned_alloc(const size_t alignment, const size_t count, const size_t size) {
            const auto alloc_bytes = count * size;

#ifdef _WIN32
            return _aligned_malloc(alloc_bytes, alignment);
#else
            return std::aligned_alloc(alignment, alloc_bytes);
#endif
        }

        void aligned_free(void* ptr) {
            if (!ptr)
            {
                return;
            }

#ifdef _WIN32
            _aligned_free(ptr);
#else
            std::free(ptr);
#endif
        }

    }

}
