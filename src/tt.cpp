#include "tt.h"
#include "search.h"

namespace sagittar {

    namespace search {

        namespace tt {

            template<typename T>
            static T* alignedAlloc(std::size_t alignment, std::size_t count) {
                const auto size = count * sizeof(T);

#ifdef _WIN32
                return static_cast<T*>(_aligned_malloc(size, alignment));
#else
                return static_cast<T*>(std::aligned_alloc(alignment, size));
#endif
            }

            static void alignedFree(void* ptr) {
                if (!ptr)
                {
                    return;
                }

#ifdef _WIN32
                _aligned_free(ptr);
#else
                std::free(ptr);
#endif
            }

            TranspositionTable::TranspositionTable(const std::size_t mb) :
                buckets(nullptr),
                size(0),
                currentage(0) {
                setSize(mb);
            }

            TranspositionTable::~TranspositionTable() { alignedFree(buckets); }

            void TranspositionTable::setSize(const std::size_t mb) {
                const std::size_t new_size = (mb * 0x100000) / sizeof(TTBucket);
                if (size != new_size)
                {
                    size = new_size;
                    if (buckets)
                    {
                        alignedFree(buckets);
                        buckets = nullptr;
                    }
                    buckets = alignedAlloc<TTBucket>(32, size);
                    if (!buckets)
                    {
                        std::cerr << "Failed to allocated memory for Transposition Table"
                                  << std::endl;
                        exit(EXIT_FAILURE);
                    }
                }
                clear();
            }

            std::size_t TranspositionTable::getSize() const { return size; }

            void TranspositionTable::clear() {
                for (u32 i = 0; i < (u32) size; i++)
                {
                    for (auto& entry : buckets[i].entries)
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
                const u64 idx         = index(hash);
                const u16 key         = hash & 0xFFFF;
                TTBucket& bucket      = buckets[idx];
                i32       min_quality = INT32_MAX;
                TTEntry*  entry_ptr   = nullptr;
                for (auto& candidate : bucket.entries)
                {
                    if (candidate.key == key || candidate.flag() == TTFlag::NONE)
                    {
                        entry_ptr = &candidate;
                        break;
                    }

                    const i32 candidate_quality = quality(candidate);
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
                        || entry.key == 0
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
                const u64       idx    = index(hash);
                const TTBucket& bucket = buckets[idx];
                const u16       key    = hash & 0xFFFF;

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
                for (u32 i = 0; i < (u32) size; i++)
                {
                    for (const auto& entry : buckets[i].entries)
                    {
                        used += (entry.flag() != TTFlag::NONE) && (entry.age() == currentage);
                    }
                }

                return (used * 1000) / (size * TTBucket::ENTRIES_PER_BUCKET);
            }

            u64 TranspositionTable::index(const u64 hash) const {
                return static_cast<u64>((static_cast<u128>(hash) * static_cast<u128>(size)) >> 64);
            }

            i32 TranspositionTable::quality(const TTEntry& entry) const {
                const i32 relative_age = (AGE_CYCLE_LENGTH + currentage - entry.age()) & AGE_MASK;
                return entry.depth - relative_age * 2;
            }

        }

    }

}
