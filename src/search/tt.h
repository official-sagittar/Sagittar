#pragma once

#include "commons/pch.h"
#include "core/move.h"
#include "core/position.h"
#include "core/types.h"

namespace sagittar::search {

    enum TTFlag : u8 {
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
            move(Move()) {}
    };

    class TranspositionTable {
       private:
        struct TTEntry {
            static constexpr u8 AGE_BITS = 5;

            u64   key;
            Score score;
            u16   move_id;
            Depth depth;
            u8    age_flag_pv;

            TTEntry() :
                key(0ULL),
                score(0),
                move_id(Move().id()),
                depth(0),
                age_flag_pv(0) {}

            u8 age() const { return static_cast<u8>(age_flag_pv >> 3); }

            TTFlag flag() const { return static_cast<TTFlag>(age_flag_pv & 3); }

            bool pv() const { return static_cast<bool>((age_flag_pv >> 2) & 1); }

            Move move() const { return Move::fromId(move_id); }

            static u8 foldAgeFlagPV(u8 age, TTFlag flag, bool pv) {
                return static_cast<u8>(flag | (pv << 2) | (age << 3));
            }
        };

        static constexpr u8 AGE_CYCLE_LEN = 1 << TTEntry::AGE_BITS;

        std::vector<TTEntry> entries;
        std::size_t          size;
        u8                   currentage;

       private:
        [[nodiscard]] inline u64 getIndex(const u64 key) const;

       public:
        explicit TranspositionTable(const std::size_t mb);
        void               setSize(const std::size_t mb);
        std::size_t        getSize() const;
        void               clear();
        void               resetForSearch();
        void               store(const u64    hash,
                                 const i32    ply,
                                 const Depth  depth,
                                 const TTFlag flag,
                                 Score        value,
                                 const Move&  move);
        [[nodiscard]] bool probe(TTData* entry, const u64 hash) const;
        u32                hashfull() const;
    };

}
