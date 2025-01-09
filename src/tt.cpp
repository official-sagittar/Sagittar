#include "tt.h"
#include "search.h"

namespace sagittar {

    namespace search {

        TranspositionTable::TranspositionTable() :
            TranspositionTable(DEFAULT_TT_SIZE_MB) {}

        TranspositionTable::TranspositionTable(const std::size_t mb) :
            size_mb(mb) {
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
            setSize(size_mb);
            currentage = 0;
        }

        void TranspositionTable::resetForSearch() { currentage++; }

        void TranspositionTable::store(const board::Board& board,
                                       const u8            depth,
                                       const TTFlag        flag,
                                       i32                 value,
                                       const move::Move    move) {
            const u64  hash  = board.getHash();
            const u64  index = hash % size;
            TTEntry&   entry = entries.at(index);
            const bool replace =
              (entry.key == 0ULL) || (entry.getAge() < currentage) || (entry.getDepth() <= depth);
            if (!replace)
            {
                return;
            }
            if (value < -MATE_SCORE)
            {
                value -= board.getPlyCount();
            }
            else if (value > MATE_SCORE)
            {
                value += board.getPlyCount();
            }
            entry             = TTEntry(hash, depth, currentage, flag, value, move);
            entries.at(index) = entry;
        }

        bool TranspositionTable::probe(TTData* data, const board::Board& board) const {
            const u64      hash  = board.getHash();
            const u64      index = hash % size;
            const TTEntry& entry = entries.at(index);
            if (entry.isValid(hash))
            {
                *data = entry.toTTData();
                return true;
            }
            return false;
        }

    }

}
