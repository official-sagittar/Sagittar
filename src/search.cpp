#include "search.h"
#include "containers.h"
#include "eval.h"
#include "movegen.h"
#include "utils.h"

namespace sagittar {

    namespace search {

        Searcher::Searcher() { stop.store(false, std::memory_order_relaxed); }

        void Searcher::shouldStopSearchNow(const SearchInfo& info) {
            if (info.timeset && (utils::currtimeInMilliseconds() >= info.stoptime))
            {
                stop.store(true, std::memory_order_relaxed);
            }
        }

        i32 Searcher::search(board::Board&     board,
                             i8                depth,
                             const SearchInfo& info,
                             SearchResult*     result) {
            if ((result->nodes & 2047) == 0)
            {
                shouldStopSearchNow(info);
            }

            const bool is_in_check = movegen::isInCheck(board);

            if (board.getPlyCount() >= MAX_DEPTH)
            {
                return eval::evaluateBoard(board);
            }
            if (is_in_check)
            {
                depth++;
            }
            if (depth <= 0)
            {
                return eval::evaluateBoard(board);
            }

            i32        max = -INF;
            move::Move bestmovesofar;
            u32        legal_moves_count = 0;

            containers::ArrayList<move::Move> moves;
            movegen::generatePseudolegalMoves(&moves, board, movegen::MovegenType::ALL);

            for (const auto& move : moves)
            {
                const board::DoMoveResult do_move_result = board.doMove(move);

                if (do_move_result == board::DoMoveResult::ILLEGAL)
                {
                    board.undoMove();
                    continue;
                }

                result->nodes++;
                legal_moves_count++;

                const i32 score = -search(board, depth - 1, info, result);

                board.undoMove();

                if (stop.load(std::memory_order_relaxed))
                {
                    return 0;
                }

                if (score > max)
                {
                    max           = score;
                    bestmovesofar = move;
                }
            }

            if (legal_moves_count == 0)
            {
                if (is_in_check)
                {
                    return -MATE_VALUE + board.getPlyCount();
                }
                else
                {
                    return 0;
                }
            }

            result->bestmove = bestmovesofar;
            return max;
        }

        SearchResult Searcher::searchRoot(
          board::Board&                                    board,
          const SearchInfo&                                info,
          std::function<void(const search::SearchResult&)> searchProgressReportHandler,
          std::function<void(const search::SearchResult&)> searchCompleteReportHander) {
            SearchResult bestresult{};

            for (u8 currdepth = 1; currdepth <= info.depth; currdepth++)
            {
                SearchResult result{};
                const u64    starttime = utils::currtimeInMilliseconds();
                i32          score     = search(board, currdepth, info, &result);
                const u64    time      = utils::currtimeInMilliseconds() - starttime;
                if (stop.load(std::memory_order_relaxed))
                {
                    break;
                }

                bestresult = result;

                result.score = score;
                if (score > -MATE_VALUE && score < -MATE_SCORE)
                {
                    result.is_mate = true;
                    result.mate_in = (-(score + MATE_VALUE) / 2 - 1);
                }
                else if (score > MATE_SCORE && score < MATE_VALUE)
                {
                    result.is_mate = true;
                    result.mate_in = ((MATE_VALUE - score) / 2 + 1);
                }
                else
                {
                    result.is_mate = false;
                    result.mate_in = 0;
                }
                result.depth = currdepth;
                result.time  = time;
                result.pv    = {result.bestmove};
                searchProgressReportHandler(result);
            }

            searchCompleteReportHander(bestresult);

            return bestresult;
        }

        void Searcher::reset() { stop.store(false, std::memory_order_relaxed); }

        SearchResult Searcher::startSearch(
          board::Board&                                    board,
          const SearchInfo&                                info,
          std::function<void(const search::SearchResult&)> searchProgressReportHandler,
          std::function<void(const search::SearchResult&)> searchCompleteReportHander) {
            return searchRoot(board, info, searchProgressReportHandler, searchCompleteReportHander);
        }

        SearchResult Searcher::startSearch(board::Board& board, const SearchInfo& info) {
            return startSearch(board, info, [](auto&) {}, [](auto&) {});
        }

        void Searcher::stopSearch() { stop.store(true, std::memory_order_relaxed); }

    }

}
