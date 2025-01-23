#pragma once

#include "board.h"
#include "move.h"
#include "pch.h"
#include "types.h"

namespace sagittar {

    namespace search {

        namespace tt {

            enum TTFlag : u8 {
                NONE,
                LOWERBOUND,
                UPPERBOUND,
                EXACT
            };

            struct TTEntry {
                u64        hash;
                i8         depth;
                u8         age;
                TTFlag     flag;
                i32        value;
                move::Move move;

                TTEntry() :
                    hash(0ULL),
                    depth(0),
                    age(0),
                    flag(TTFlag::NONE),
                    value(0),
                    move(move::Move()) {}

                TTEntry(const u64        hash,
                        const i8         depth,
                        const u8         age,
                        const TTFlag     flag,
                        const i32        value,
                        const move::Move move) :
                    hash(hash),
                    depth(depth),
                    age(age),
                    flag(flag),
                    value(value),
                    move(move) {}
            };

            class TranspositionTable {
               private:
                std::vector<TTEntry> entries;
                std::size_t          size;
                u8                   currentage;

               public:
                explicit TranspositionTable(const std::size_t mb);
                void               setSize(const std::size_t mb);
                std::size_t        getSize() const;
                void               clear();
                void               resetForSearch();
                void               store(const board::Board& board,
                                         const i8            depth,
                                         const TTFlag        flag,
                                         i32                 value,
                                         const move::Move    move);
                [[nodiscard]] bool probe(TTEntry* entry, const board::Board& board) const;
            };

        }

    }

}
