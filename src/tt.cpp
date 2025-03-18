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
                size = (mb * 1024 * 1024) / sizeof(TTBucket);
                buckets.clear();
                buckets.resize(size);
                buckets.shrink_to_fit();
                currentage = 0;
            }

            std::size_t TranspositionTable::getSize() const { return size; }

            void TranspositionTable::clear() {
                for (auto& bucket : buckets)
                {
                    for (auto& entry : bucket.entries)
                    {
                        entry = TTEntry();
                    }
                }
                currentage = 0;
            }

            void TranspositionTable::resetForSearch() {
                currentage = (currentage + 1) % AGE_CYCLE_LENGTH;
            }

            void TranspositionTable::store(const u64        hash,
                                           const i32        ply,
                                           const Depth      depth,
                                           const TTFlag     flag,
                                           Score            value,
                                           const move::Move move) {
                const u64 index       = hash % size;
                const u16 key         = hash & 0xFFFF;
                TTBucket& bucket      = buckets.at(index);
                i32       min_quality = INT32_MAX;
                TTEntry*  entry_ptr   = nullptr;
                for (auto& candidate : bucket.entries)
                {
                    if (candidate.key == key || candidate.flag() == TTFlag::NONE)
                    {
                        entry_ptr = &candidate;
                        break;
                    }

                    i32 candidate_quality = quality(candidate.age(), candidate.depth);
                    if (candidate_quality < min_quality)
                    {
                        entry_ptr   = &candidate;
                        min_quality = candidate_quality;
                    }
                }

#ifdef DEBUG
                assert(entry_ptr != nullptr);
#endif

                TTEntry entry = *entry_ptr;

                // Replacement scheme
                // clang-format off
                if (!(flag == TTFlag::EXACT
                        || key != entry.key
                        || currentage != entry.age()
                        || depth + 4 > entry.depth))
                {
                    return;
                }
                // clang-format on

                // Only if current entry is from a different position OR if move is not a null move,
                // Replace the move in the entry
                if (move != move::Move() || entry.key != key)
                {
                    entry.move_id = move.id();
                }

                if (value < -MATE_SCORE)
                {
                    value -= ply;
                }
                else if (value > MATE_SCORE)
                {
                    value += ply;
                }

                entry.key         = key;
                entry.score       = value;
                entry.static_eval = value;
                entry.depth       = depth;
                entry.age_flag_pv = TTEntry::foldAgeFlagPV(currentage, flag, false);

                *entry_ptr = entry;
            }

            bool TranspositionTable::probe(TTData* ttdata, const u64 hash) const {
                const u64      index  = hash % size;
                const TTBucket bucket = buckets.at(index);
                const u16      key    = hash & 0xFFFF;

                for (const auto& entry : bucket.entries)
                {
                    if (entry.key == key)
                    {
                        ttdata->depth = entry.depth;
                        ttdata->flag  = entry.flag();
                        ttdata->score = entry.score;
                        ttdata->move  = entry.move();
                        return true;
                    }
                }

                return false;
            }

            u32 TranspositionTable::hashfull() const {
                u32 used = 0;
                for (const auto& bucket : buckets)
                {
                    for (const auto& entry : bucket.entries)
                    {
                        used += (entry.flag() != TTFlag::NONE) && (entry.age() == currentage);
                    }
                }

                return (used * 1000) / (size * TTBucket::ENTRIES_PER_BUCKET);
            }

            i32 TranspositionTable::quality(const u8 age, const Depth depth) const {
                const i32 relative_age = (AGE_CYCLE_LENGTH + currentage - age) & AGE_MASK;
                return depth - 2 * relative_age;
            }

        }

    }

}
