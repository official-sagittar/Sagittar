#pragma once

#include "core/defs.h"
#include "pch.h"

namespace sagittar {

    namespace core {

        constexpr size_t TT_SIZE_DEFAULT = 16;

        struct TTEntry {
            uint64_t key;
            int      depth;
            int      value;
        };

        class TranspositionTable {
           public:
            TranspositionTable();
            TranspositionTable(const size_t size_mb);
            void clear();
            void resize(const size_t mb);
            void store(const uint64_t key, const int depth, const int value);
            bool probe(TTEntry* const out, const uint64_t key) const;
            ~TranspositionTable();

           private:
            TTEntry* entries;
            size_t   size;
        };

    }

}
