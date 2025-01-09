#pragma once

#include "board.h"
#include "move.h"
#include "pch.h"
#include "types.h"

namespace sagittar {

    namespace search {

        enum TTFlag : u8 {
            NONE,
            LOWERBOUND,
            UPPERBOUND,
            EXACT
        };

        struct TTData {
            u8         depth;
            TTFlag     flag;
            i32        value;
            move::Move move;
        };

        struct TTEntry {
            u64 key;
            u64 data;

            TTEntry() :
                key(0ULL),
                data(0ULL) {}

            TTEntry(const u64        hash,
                    const u8         depth,
                    const u8         age,
                    const TTFlag     flag,
                    const i32        value,
                    const move::Move move) {
                data = (static_cast<u64>(move.id()) << 48) | (value << 16) | (flag << 14)
                     | (age << 6) | depth;
                key = hash ^ data;
            }

            u8 getDepth() const { return static_cast<u8>(data & 0x3F); }

            u8 getAge() const { return static_cast<u8>((data >> 6) & 0xFF); }

            move::Move getMove() const { return move::Move::fromId((data >> 48) & 0xFFFF); }

            bool isValid(const u64 hash) const { return (hash ^ data) == key; }

            TTData toTTData() const {
                TTData ttdata;
                ttdata.depth = getDepth();
                ttdata.flag  = static_cast<TTFlag>((data >> 14) & 0x3);
                ttdata.value = static_cast<i32>((data >> 16) & 0xFFFFFFFF);
                ttdata.move  = move::Move::fromId((data >> 48) & 0xFFFF);
                return ttdata;
            }
        };

        class TranspositionTable {
           private:
            std::vector<TTEntry> entries;
            std::size_t          size_mb;
            std::size_t          size;
            u8                   currentage;

           public:
            TranspositionTable();
            TranspositionTable(const std::size_t mb);
            void               setSize(const std::size_t mb);
            std::size_t        getSize() const;
            void               clear();
            void               resetForSearch();
            void               store(const board::Board& board,
                                     const u8            depth,
                                     const TTFlag        flag,
                                     const i32           value,
                                     const move::Move    move);
            [[nodiscard]] bool probe(TTData* data, const board::Board& board) const;
        };

    }

}
