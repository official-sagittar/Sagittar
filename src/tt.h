#pragma once

#include "move.h"
#include "pch.h"
#include "types.h"

namespace sagittar {

    namespace search {

        enum class TTFlag : u8 {
            NONE,
            LOWERBOUND,
            UPPERBOUND,
            EXACT
        };

        struct TTEntry {
            u64 key;
            u64 data;

            TTEntry() :
                key(0ULL),
                data(0ULL) {}

            TTEntry(const u64 key, const u64 data) :
                key(key),
                data(data) {}
        };

        class TranspositionTable {
           private:
            std::vector<TTEntry> entries;
            std::size_t          size;
            u32                  currentage;

           public:
            TranspositionTable(const std::size_t mb);
            void        setSize(const std::size_t mb);
            std::size_t getSize() const;
            void        clear();
            void        resetForSearch();
            void        store(const u64        hash,
                              const u8         depth,
                              const TTFlag     flag,
                              const i32        value,
                              const move::Move move);
            bool        probe() const;
        };

    }

}
