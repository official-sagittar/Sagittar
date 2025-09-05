#include "tt.h"
#include "arch.h"
#include "core/utils.h"

namespace sagittar {

    namespace core {

        TranspositionTable::TranspositionTable() :
            entries(nullptr),
            size(0) {
            resize(TT_SIZE_DEFAULT);
        }

        TranspositionTable::TranspositionTable(const size_t size_mb) :
            entries(nullptr),
            size(size_mb) {
            resize(size_mb);
        }

        void TranspositionTable::clear() {
            for (size_t i = 0; i < size; i++)
            {
                entries[i] = TTEntry();
            }
        }

        void TranspositionTable::resize(const size_t size_mb) {
            const auto   bytes    = size_mb * 1024 * 1024;
            const size_t new_size = bytes / sizeof(TTEntry);
            if (new_size != size)
            {
                size = new_size;
                if (entries)
                {
                    aligned_free(entries);
                }
                entries = static_cast<TTEntry*>(
                  aligned_alloc(DEFAULT_CACHE_LINE_SIZE, bytes, sizeof(TTEntry)));
                if (!entries)
                {
                    std::cout << "info string Failed to allocate Tranposition Table memory!"
                              << std::endl;
                    std::terminate();
                }
            }
            clear();
        }

        void TranspositionTable::store(const uint64_t key, const int depth, const int value) {
            const size_t   index = key % size;
            TTEntry* const entry = &entries[index];

            const bool replace = (entry->key != key) || (entry->depth != depth);
            if (!replace)
            {
                return;
            }

            TTEntry new_entry;

            new_entry.key   = key;
            new_entry.depth = depth;
            new_entry.value = value;

            *entry = new_entry;
        }

        bool TranspositionTable::probe(TTEntry* const out, const uint64_t key) const {
            const size_t         index = key % size;
            const TTEntry* const entry = &entries[index];
            if (entry->key == key)
            {
                *out = *entry;
                return true;
            }
            return false;
        }

        TranspositionTable::~TranspositionTable() {
            if (entries)
                aligned_free(entries);
        }
    }

}
