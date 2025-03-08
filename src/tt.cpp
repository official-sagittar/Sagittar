#include "tt.h"
#include "search.h"

namespace sagittar {

    namespace search {

        namespace tt {

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
                for (u32 i = 0; i < size; i++)
                {
                    entries.at(i) = TTEntry();
                }
                currentage = 0;
            }

            void TranspositionTable::resetForSearch() { currentage++; }

            void TranspositionTable::store(const u64        hash,
                                           const i32        ply,
                                           const i8         depth,
                                           const TTFlag     flag,
                                           i32              value,
                                           const move::Move move) {
                const u64     index     = hash % size;
                const TTEntry currentry = entries.at(index);

                // Only handles empty indices or stale entires
                const bool replace = (currentry.hash == 0ULL) || (currentry.age < currentage)
                                  || (currentry.depth <= depth);
                if (!replace)
                {
                    return;
                }

                if (value < -MATE_SCORE)
                {
                    value -= ply;
                }
                else if (value > MATE_SCORE)
                {
                    value += ply;
                }

                // If current entry is from the current position AND if move is a null move,
                // DO NOT replace the move in the entry
                move::Move move_to_replace = move;
                if ((move == move::Move()) && (currentry.hash == hash))
                {
                    move_to_replace = currentry.move;
                }

                const TTEntry newentry =
                  TTEntry(hash, depth, currentage, flag, value, move_to_replace);

                entries.at(index) = newentry;
            }

            bool TranspositionTable::probe(TTEntry* entry, const u64 hash) const {
                const u64     index     = hash % size;
                const TTEntry currentry = entries.at(index);

                if (currentry.hash == hash)
                {
                    entry->hash  = currentry.hash;
                    entry->depth = currentry.depth;
                    entry->age   = currentry.age;
                    entry->flag  = currentry.flag;
                    entry->value = currentry.value;
                    entry->move  = currentry.move;
                    return true;
                }

                return false;
            }

            u32 TranspositionTable::hashfull() const {
                u32 used = 0;
                for (auto& e : entries)
                {
                    used += (e.flag != TTFlag::NONE) && (e.age == currentage);
                }
                return used * 1000 / size;
            }

        }

    }

}
