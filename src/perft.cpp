#include "perft.h"
#include "containers.h"
#include "move.h"
#include "movegen.h"

namespace sagittar {

    namespace perft {

        u64 perft(board::Board& board, const Depth depth) {
            if (depth == 0)
            {
                return 1ULL;
            }
            u64                               nodes = 0ULL;
            containers::ArrayList<move::Move> moves;
            movegen::generatePseudolegalMoves(&moves, board, movegen::MovegenType::ALL);
            for (auto const& move : moves)
            {
                const board::DoMoveResult result = board.doMove(move);
                if (result == board::DoMoveResult::LEGAL)
                {
                    nodes += perft(board, depth - 1);
                }
                board.undoMove();
            }
            return nodes;
        }

        u64 divide(board::Board& board, const Depth depth) {
            if (depth == 0)
            {
                return 1ULL;
            }
            u64                               nodes       = 0ULL;
            u64                               total_nodes = 0ULL;
            containers::ArrayList<move::Move> moves;
            movegen::generatePseudolegalMoves(&moves, board, movegen::MovegenType::ALL);
            for (auto const& move : moves)
            {
                const board::DoMoveResult result = board.doMove(move);
                if (result == board::DoMoveResult::LEGAL)
                {
                    nodes = perft(board, depth - 1);
                    total_nodes += nodes;
                    move.display();
                    std::cout << " " << (u64) nodes << std::endl;
                }
                board.undoMove();
            }
            std::cout << std::endl << "Perft = " << (u64) total_nodes << std::endl;
            return total_nodes;
        }

    }

}
