#include "perft.h"
#include "commons/containers.h"
#include "core/move.h"
#include "core/movegen.h"

namespace sagittar::perft {

    size_t perft(const Position& pos, const Depth depth) {
        if (depth == 0)
        {
            return 1ULL;
        }
        size_t                      nodes = 0ULL;
        containers::ArrayList<Move> moves;
        pseudolegalMoves<MovegenType::ALL>(&moves, pos);
        for (auto const& move : moves)
        {
            Position pos_copy = pos;
            if (pos_copy.doMove(move))
            {
                nodes += perft(pos_copy, depth - 1);
            }
        }
        return nodes;
    }

    size_t divide(const Position& pos, const Depth depth) {
        if (depth == 0)
        {
            return 1ULL;
        }
        size_t                      nodes       = 0ULL;
        size_t                      total_nodes = 0ULL;
        containers::ArrayList<Move> moves;
        pseudolegalMoves<MovegenType::ALL>(&moves, pos);
        for (auto const& move : moves)
        {
            Position pos_copy = pos;
            if (pos_copy.doMove(move))
            {
                nodes = perft(pos_copy, depth - 1);
                total_nodes += nodes;
                move.display();
                std::cout << " " << (size_t) nodes << std::endl;
            }
        }
        std::cout << std::endl << "Perft = " << (size_t) total_nodes << std::endl;
        return total_nodes;
    }

}
