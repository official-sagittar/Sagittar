#include "perft.h"
#include "core/move.h"
#include "core/movegen.h"

namespace sagittar {

    namespace core {

        uint64_t perft(const Position&                                                pos,
                       const int                                                      depth,
                       TranspositionTable<TTClient::PERFT, uint64_t, uint32_t>* const tt,
                       PositionHistory*                                               history) {
            if (depth == 0)
            {
                return 1ULL;
            }

            TTData<uint32_t> ttdata;
            if (tt->probe(&ttdata, pos.hash, 0) && ttdata.depth == depth)
            {
                return ttdata.value;
            }

            uint64_t nodes      = 0ULL;
            MoveList moves_list = {};
            movegen_generate_pseudolegal_moves<MovegenType::MOVEGEN_ALL>(pos, &moves_list);
            for (auto [move, score] : moves_list)
            {
                Position pos_dup = pos;
                if (pos_dup.do_move(move, history))
                {
                    nodes += perft(pos_dup, depth - 1, tt, history);
                }
                pos_dup.undo_move(history);
            }

            tt->store(pos.hash, depth, 0, TT_FLAG_EXACT, nodes, NULL_MOVE);

            return nodes;
        }

        uint64_t divide(const Position&                                                pos,
                        const int                                                      depth,
                        TranspositionTable<TTClient::PERFT, uint64_t, uint32_t>* const tt,
                        PositionHistory*                                               history) {
            if (depth == 0)
            {
                return 1ULL;
            }
            uint64_t total_nodes = 0ULL;
            MoveList moves_list  = {};
            movegen_generate_pseudolegal_moves<MovegenType::MOVEGEN_ALL>(pos, &moves_list);
            for (auto [move, score] : moves_list)
            {
                Position pos_dup = pos;
                if (pos_dup.do_move(move, history))
                {
                    uint64_t nodes = perft(pos_dup, depth - 1, tt, history);
                    total_nodes += nodes;
                    std::cout << move_tostring(move) << " " << (uint64_t) nodes << std::endl;
                }
                pos_dup.undo_move(history);
            }
            std::cout << "\nNodes = " << (uint64_t) total_nodes << std::endl;
            return total_nodes;
        }

    }

}
