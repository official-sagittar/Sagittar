#include "tt.h"

namespace sagittar {

    namespace search {

        TranspositionTable::TranspositionTable(const std::size_t mb) {
            setSize(mb);
            currentage = 0;
        }

        void TranspositionTable::setSize(const std::size_t mb) {
            size = (mb * 0x100000) / sizeof(TTEntry);
            size -= sizeof(TTEntry);
            entries.clear();
            entries.resize(size);
            entries.shrink_to_fit();
        }

        std::size_t TranspositionTable::getSize() const { return size; }

        void TranspositionTable::clear() {
            entries.clear();
            currentage = 0;
        }

        void TranspositionTable::resetForSearch() { currentage++; }

        void TranspositionTable::store(const u64        hash,
                                       const u8         depth,
                                       const TTFlag     flag,
                                       const i32        value,
                                       const move::Move move) {

            const u64 index = hash % size;
            const u64 data  = 0ULL;
            const u64 key   = hash ^ data;

            entries[index].key  = key;
            entries[index].data = data;
        }

    }

}
