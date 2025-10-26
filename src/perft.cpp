#include "perft.h"
#include "containers.h"
#include "move.h"
#include "movegen.h"

namespace sagittar {

    namespace perft {

        u64 perft(const core::Position& pos, const Depth depth) {
            if (depth == 0)
            {
                return 1ULL;
            }
            u64                               nodes = 0ULL;
            containers::ArrayList<move::Move> moves;
            movegen::generatePseudolegalMoves<movegen::MovegenType::ALL>(&moves, pos);
            for (auto const& move : moves)
            {
                core::Position           pos_copy = pos;
                const core::DoMoveResult result   = pos_copy.doMove(move);
                if (result == core::DoMoveResult::LEGAL)
                {
                    nodes += perft(pos_copy, depth - 1);
                }
            }
            return nodes;
        }

        u64 divide(const core::Position& pos, const Depth depth) {
            if (depth == 0)
            {
                return 1ULL;
            }
            u64                               nodes       = 0ULL;
            u64                               total_nodes = 0ULL;
            containers::ArrayList<move::Move> moves;
            movegen::generatePseudolegalMoves<movegen::MovegenType::ALL>(&moves, pos);
            for (auto const& move : moves)
            {
                core::Position           pos_copy = pos;
                const core::DoMoveResult result   = pos_copy.doMove(move);
                if (result == core::DoMoveResult::LEGAL)
                {
                    nodes = perft(pos_copy, depth - 1);
                    total_nodes += nodes;
                    move.display();
                    std::cout << " " << (u64) nodes << std::endl;
                }
            }
            std::cout << std::endl << "Perft = " << (u64) total_nodes << std::endl;
            return total_nodes;
        }

    }

}
