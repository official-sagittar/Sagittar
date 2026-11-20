#pragma once

#include "commons/pch.h"
#include "core/move.h"
#include "core/position.h"
#include "core/types.h"

namespace sagittar::search {

    enum TTFlag : U8 {
        NONE,
        LOWERBOUND,
        UPPERBOUND,
        EXACT
    };

    struct TTData {
        Depth  depth;
        TTFlag flag;
        Score  score;
        Move   move;

        TTData() :
            depth(0),
            flag(TTFlag::NONE),
            score(0),
            move(Move()) { }
    };

    class TranspositionTable {
       private:
        struct TTEntry {
            static constexpr U8 k_AGE_BITS = 5;

            U64 key;
            I16 score;
            U16 move_id;
            I8  depth;
            U8  age_flag_pv;

            TTEntry() :
                key(0ULL),
                score(0),
                move_id(Move().id()),
                depth(0),
                age_flag_pv(0) { }

            U8 age() const { return static_cast<U8>(age_flag_pv >> 3); }

            TTFlag flag() const { return static_cast<TTFlag>(age_flag_pv & 3); }

            bool pv() const { return static_cast<bool>((age_flag_pv >> 2) & 1); }

            Move move() const { return Move(move_id); }

            static U8 foldAgeFlagPV(U8 age, TTFlag flag, bool pv) {
                return static_cast<U8>(flag | (pv << 2) | (age << 3));
            }
        };

        static constexpr U8 k_AGE_CYCLE_LEN = 1 << TTEntry::k_AGE_BITS;

        std::vector<TTEntry> m_entries;
        std::size_t          m_size;
        U8                   m_currentage;

       private:
        [[nodiscard]] inline U64 getIndex(const U64 key) const;

       public:
        explicit TranspositionTable(const std::size_t mb);
        void               setSize(const std::size_t mb);
        std::size_t        getSize() const;
        void               clear();
        void               resetForSearch();
        void               store(const U64    hash,
                                 const I32    ply,
                                 const Depth  depth,
                                 const TTFlag flag,
                                 Score        value,
                                 const Move&  move);
        [[nodiscard]] bool probe(TTData* entry, const U64 hash) const;
        U32                hashfull() const;
    };

}
