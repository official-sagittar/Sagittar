#include "tt.h"
#include "search.h"

namespace sagittar {

    namespace search {

        namespace tt {

            TranspositionTable::TranspositionTable(const std::size_t mb) :
                size(0),
                currentage(0) {
                setSize(mb);
            }

            void TranspositionTable::setSize(const std::size_t mb) {
                size = (mb * 0x100000) / sizeof(TTEntry);
                size -= sizeof(TTEntry);
                entries.clear();
                entries.resize(size);
                entries.shrink_to_fit();
                currentage = 0;
            }

            std::size_t TranspositionTable::getSize() const { return size; }

            void TranspositionTable::clear() {
                for (u32 i = 0; i < size; i++)
                {
                    entries.at(i) = TTEntry();
                }
                currentage = 0;
            }

            void TranspositionTable::resetForSearch() {
                currentage = (currentage + 1) % AGE_CYCLE_LEN;
            }

            void TranspositionTable::store(const u64        hash,
                                           const i32        ply,
                                           const Depth      depth,
                                           const TTFlag     flag,
                                           Score            value,
                                           const move::Move& move) {
                const u64     index     = getIndex(hash);
                const TTEntry currentry = entries.at(index);

                // Only handles empty indices or stale entires
                const bool replace = (currentry.key == 0ULL) || (currentry.age() != currentage)
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
                if ((move == move::NULL_MOVE) && (currentry.key == hash))
                {
                    move_to_replace = currentry.move();
                }

                TTEntry newentry;
                newentry.key         = hash;
                newentry.score       = static_cast<i16>(value);
                newentry.move_id     = move_to_replace.id();
                newentry.depth       = static_cast<u8>(depth);
                newentry.age_flag_pv = TTEntry::foldAgeFlagPV(currentage, flag, false);

                entries.at(index) = newentry;
            }

            bool TranspositionTable::probe(TTData* ttdata, const u64 hash) const {
                const u64     index     = getIndex(hash);
                const TTEntry currentry = entries.at(index);

                if (currentry.key == hash)
                {
                    ttdata->depth = currentry.depth;
                    ttdata->flag  = currentry.flag();
                    ttdata->score = currentry.score;
                    ttdata->move  = currentry.move();
                    return true;
                }

                return false;
            }

            u32 TranspositionTable::hashfull() const {
                u32 used = 0;
                for (u16 i = 0; i < 1000; i++)
                {
                    const TTEntry& e = entries.at(i);
                    used += (e.flag() != TTFlag::NONE) && (e.age() == currentage);
                }
                return used;
            }

            [[nodiscard]] inline u64 TranspositionTable::getIndex(const u64 key) const {
                // this emits a single mul on both x64 and arm64
                return static_cast<u64>((static_cast<u128>(key) * static_cast<u128>(size)) >> 64);
            }

        }

    }

}
