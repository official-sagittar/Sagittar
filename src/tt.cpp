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
                size = (mb * 0x100000) / sizeof(TTBucket);
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
                        entry = Entry();
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
                Entry*    entry_ptr   = nullptr;
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

                Entry entry = *entry_ptr;

                // Replacement scheme
                const bool replace = (entry.age() < currentage) || (entry.depth <= depth);
                if (!replace)
                {
                    return;
                }

                // If move is a null move or current entry is from a different position, replace move
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
                entry.static_eval = value;
                entry.depth       = depth;
                entry.score       = value;
                entry.age_flag_pv = Entry::foldAgeFlagPV(currentage, flag, false);

                *entry_ptr = entry;
            }

            bool TranspositionTable::probe(TTData* ttdata, const u64 hash) const {
                const u64 index  = hash % size;
                TTBucket  bucket = buckets.at(index);
                const u16 key    = hash & 0xFFFF;

                for (auto& entry : bucket.entries)
                {
                    if (entry.key == key)
                    {
                        ttdata->score       = entry.score;
                        ttdata->static_eval = entry.static_eval;
                        ttdata->move        = move::Move::fromId(entry.move_id);
                        ttdata->depth       = entry.depth;
                        ttdata->pv          = entry.pv();
                        ttdata->flag        = entry.flag();

                        return true;
                    }
                }

                return false;
            }

            u32 TranspositionTable::hashfull() const {
                u32 used = 0;
                for (auto& bucket : buckets)
                {
                    for (auto& entry : bucket.entries)
                    {
                        used += (entry.flag() != TTFlag::NONE) && (entry.age() == currentage);
                    }
                }
                return used * 1000 / size;
            }

            i32 TranspositionTable::quality(const u8 age, const Depth depth) const {
                const i32 relative_age = (AGE_CYCLE_LENGTH + currentage - age) & AGE_MASK;
                return depth - 2 * relative_age;
            }

        }

    }

}
