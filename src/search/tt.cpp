#include "tt.h"
#include "search/search.h"

namespace sagittar::search {

    TranspositionTable::TranspositionTable(const std::size_t mb) :
        m_size(0),
        m_currentage(0) {
        setSize(mb);
    }

    void TranspositionTable::setSize(const std::size_t mb) {
        m_size = (mb * 0x100000) / sizeof(TTEntry);
        m_size -= sizeof(TTEntry);
        m_entries.clear();
        m_entries.resize(m_size);
        m_entries.shrink_to_fit();
        m_currentage = 0;
    }

    std::size_t TranspositionTable::getSize() const { return m_size; }

    void TranspositionTable::clear() {
        for (U32 i = 0; i < m_size; i++)
        {
            m_entries.at(i) = TTEntry();
        }
        m_currentage = 0;
    }

    void TranspositionTable::resetForSearch() {
        m_currentage = (m_currentage + 1) % k_AGE_CYCLE_LEN;
    }

    void TranspositionTable::store(const U64    hash,
                                   const I32    ply,
                                   const Depth  depth,
                                   const TTFlag flag,
                                   Score        value,
                                   const Move&  move) {
        const U64     index     = getIndex(hash);
        const TTEntry currentry = m_entries.at(index);

        // Only handles empty indices or stale entires
        const bool replace = (currentry.key == 0ULL) || (currentry.age() != m_currentage)
                          || (currentry.depth <= depth);
        if (!replace)
        {
            return;
        }

        if (value < -k_MATE_SCORE)
        {
            value -= ply;
        }
        else if (value > k_MATE_SCORE)
        {
            value += ply;
        }

        // If current entry is from the current position AND if move is a null move,
        // DO NOT replace the move in the entry
        Move move_to_replace = move;
        if ((move == Move()) && (currentry.key == hash))
        {
            move_to_replace = currentry.move();
        }

        TTEntry newentry;
        newentry.key         = hash;
        newentry.score       = static_cast<I16>(value);
        newentry.move_id     = move_to_replace.id();
        newentry.depth       = static_cast<I8>(depth);
        newentry.age_flag_pv = TTEntry::foldAgeFlagPV(m_currentage, flag, false);

        m_entries.at(index) = newentry;
    }

    bool TranspositionTable::probe(TTData* ttdata, const U64 hash) const {
        const U64     index     = getIndex(hash);
        const TTEntry currentry = m_entries.at(index);

        if (currentry.key == hash)
        {
            ttdata->depth = static_cast<Depth>(currentry.depth);
            ttdata->flag  = currentry.flag();
            ttdata->score = static_cast<Score>(currentry.score);
            ttdata->move  = currentry.move();
            return true;
        }

        return false;
    }

    U32 TranspositionTable::hashfull() const {
        U32 used = 0;
        for (U16 i = 0; i < 1000; i++)
        {
            const TTEntry& e = m_entries.at(i);
            used += (e.flag() != TTFlag::NONE) && (e.age() == m_currentage);
        }
        return used;
    }

    [[nodiscard]] inline U64 TranspositionTable::getIndex(const U64 key) const {
        // this emits a single mul on both x64 and arm64
        return static_cast<U64>((static_cast<U128>(key) * static_cast<U128>(m_size)) >> 64);
    }

}
