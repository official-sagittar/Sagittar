#pragma once

#include "core/defs.h"
#include "core/move.h"
#include "pch.h"

namespace sagittar {

    namespace core {

        constexpr size_t TT_SIZE_DEFAULT = 16;

        template<typename T>
        concept KeyType = std::is_same_v<T, uint64_t>;

        template<typename T>
        concept ValueType = std::is_same_v<T, Score> || std::is_same_v<T, uint32_t>;

        enum class TTClient {
            PERFT,
            SEARCH
        };

        enum TTFlag : uint8_t {
            TT_FLAG_NONE,
            TT_FLAG_EXACT,
            TT_FLAG_LOWERBOUND,
            TT_FLAG_UPPERBOUND
        };

        template<ValueType V>
        struct TTData {
            uint8_t depth;
            TTFlag  flag;
            V       value;
            Move    move;

            TTData() :
                depth(0),
                flag(TT_FLAG_NONE),
                value(0),
                move(NULL_MOVE) {}
        };

        template<TTClient C, KeyType K, ValueType V>
        class TranspositionTable {
           public:
            TranspositionTable();
            TranspositionTable(const size_t size_mb);
            void clear();
            void resize(const size_t mb);
            void store(const uint64_t key,
                       const int      depth,
                       const int      ply,
                       const TTFlag   flag,
                       const V        value,
                       const Move     move);
            bool probe(TTData<V>* const out, const uint64_t key, const int ply) const;
            ~TranspositionTable();

           private:
            struct TTEntry {
                K       key;
                uint8_t depth;
                TTFlag  flag;
                V       value;
                Move    move;

                TTEntry() :
                    key(0),
                    depth(0),
                    flag(TT_FLAG_NONE),
                    value(0),
                    move(NULL_MOVE) {}

                TTEntry(
                  const K key, const int depth, const TTFlag flag, const V value, const Move move) :
                    key(key),
                    depth(depth),
                    flag(flag),
                    value(value),
                    move(move) {}
            };

            TTEntry* entries;
            size_t   size;
        };

    }

}
