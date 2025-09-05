#include "perft.h"
#include "core/move.h"
#include "core/movegen.h"

namespace sagittar {

    namespace core {

        uint64_t perft(Position* const pos, const int depth, TranspositionTable* const tt) {
            if (depth == 0)
            {
                return 1ULL;
            }

            TTEntry entry;
            if (tt->probe(&entry, pos->hash) && entry.depth == depth)
            {
                return entry.value;
            }

            uint64_t nodes      = 0ULL;
            MoveList moves_list = {};
            movegen_generate_pseudolegal_moves<MovegenType::MOVEGEN_ALL>(pos, &moves_list);
            for (auto [move, score] : moves_list)
            {
                Position pos_dup = *pos;
                if (pos_dup.do_move(move))
                {
                    nodes += perft(&pos_dup, depth - 1, tt);
                }
            }

            tt->store(pos->hash, depth, nodes);

            return nodes;
        }

        uint64_t divide(Position* const pos, const int depth, TranspositionTable* const tt) {
            if (depth == 0)
            {
                return 1ULL;
            }
            uint64_t total_nodes = 0ULL;
            MoveList moves_list  = {};
            movegen_generate_pseudolegal_moves<MovegenType::MOVEGEN_ALL>(pos, &moves_list);
            for (auto [move, score] : moves_list)
            {
                Position pos_dup = *pos;
                if (pos_dup.do_move(move))
                {
                    uint64_t nodes = perft(&pos_dup, depth - 1, tt);
                    total_nodes += nodes;
                    std::cout << move_tostring(move) << " " << (uint64_t) nodes << std::endl;
                }
            }
            std::cout << "\nNodes = " << (uint64_t) total_nodes << std::endl;
            return total_nodes;
        }

    }

}
