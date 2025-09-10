#include "tt.h"
#include "arch.h"
#include "core/utils.h"

namespace sagittar {

    namespace core {

        template<ValueType V>
        static constexpr V score_to_tt(const V val, const int ply) {
            const bool is_mate_score_neg = val < (-MATE_SCORE + DEPTH_MAX);
            const bool is_mate_score_pos = val > (MATE_SCORE - DEPTH_MAX);

            return val - (static_cast<V>(is_mate_score_neg * ply))
                 + (static_cast<V>(is_mate_score_pos * ply));
        }

        template<ValueType V>
        static constexpr V score_from_tt(const V val, const int ply) {
            const bool is_mate_score_neg = val < (-MATE_SCORE + DEPTH_MAX);
            const bool is_mate_score_pos = val > (MATE_SCORE - DEPTH_MAX);

            return val + (static_cast<V>(is_mate_score_neg * ply))
                 - (static_cast<V>(is_mate_score_pos * ply));
        }

        template<TTClient C, KeyType K, ValueType V>
        TranspositionTable<C, K, V>::TranspositionTable() :
            entries(nullptr),
            size(0) {
            resize(TT_SIZE_DEFAULT);
        }

        template<TTClient C, KeyType K, ValueType V>
        TranspositionTable<C, K, V>::TranspositionTable(const size_t size_mb) :
            entries(nullptr),
            size(size_mb) {
            resize(size_mb);
        }

        template<TTClient C, KeyType K, ValueType V>
        void TranspositionTable<C, K, V>::clear() {
            for (size_t i = 0; i < size; i++)
            {
                entries[i] = TTEntry();
            }
        }

        template<TTClient C, KeyType K, ValueType V>
        void TranspositionTable<C, K, V>::resize(const size_t size_mb) {
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

        template<TTClient C, KeyType K, ValueType V>
        void TranspositionTable<C, K, V>::store(const uint64_t key,
                                                const int      depth,
                                                const int      ply,
                                                const TTFlag   flag,
                                                const V        value,
                                                const Move     move) {

            assert(depth >= 0 && depth < DEPTH_MAX);

            const size_t   index = key % size;
            TTEntry* const entry = &entries[index];

            const bool replace = (entry->key != key) || (entry->depth != depth);
            if (!replace)
                return;

            V adjusted_val;
            if constexpr (C == TTClient::SEARCH)
                adjusted_val = score_to_tt(value, ply);
            else
                adjusted_val = value;

            *entry = TTEntry(key, static_cast<uint8_t>(depth), flag, adjusted_val, move);
        }

        template<TTClient C, KeyType K, ValueType V>
        bool TranspositionTable<C, K, V>::probe(TTData<V>* const out,
                                                const uint64_t   key,
                                                const int        ply) const {
            const size_t         index = key % size;
            const TTEntry* const entry = &entries[index];
            if (entry->key == key)
            {
                out->depth = entry->depth;
                out->flag  = entry->flag;
                if constexpr (C == TTClient::SEARCH)
                    out->value = score_from_tt(entry->value, ply);
                else
                    out->value = entry->value;
                out->move = entry->move;
                return true;
            }
            return false;
        }

        template<TTClient C, KeyType K, ValueType V>
        TranspositionTable<C, K, V>::~TranspositionTable() {
            if (entries)
                aligned_free(entries);
        }

        template class TranspositionTable<TTClient::PERFT, uint64_t, uint32_t>;
        template class TranspositionTable<TTClient::SEARCH, uint64_t, Score>;
    }
}
