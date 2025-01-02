#include "search.h"
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

        i32 Searcher::search(board::Board& board, const SearchInfo& info, SearchResult* result) {
            if ((result->nodes & 2047) == 0)
            {
                shouldStopSearchNow(info);
            }
            std::vector<move::Move> moves;
            movegen::generatePseudolegalMoves(&moves, board, movegen::MovegenType::ALL);
            result->bestmove = moves.at(0);
            for (const auto& move : moves)
            {
                const board::DoMoveResult do_move_result = board.doMove(move);
                if (do_move_result == board::DoMoveResult::ILLEGAL)
                {
                    board.undoMove();
                    continue;
                }
                else if (do_move_result == board::DoMoveResult::INVALID)
                {
                    continue;
                }
                result->nodes++;
                board.undoMove();
                if (stop.load(std::memory_order_relaxed))
                {
                    return eval::evaluateBoard(board);
                }
                result->bestmove = move;
                break;
            }
            return eval::evaluateBoard(board);
        }

        SearchResult Searcher::searchRoot(
          board::Board&                                    board,
          const SearchInfo&                                info,
          std::function<void(const search::SearchResult&)> searchProgressReportHandler,
          std::function<void(const search::SearchResult&)> searchCompleteReportHander) {
            SearchResult result{};

            const u64 starttime = utils::currtimeInMilliseconds();
            i32       score     = search(board, info, &result);
            const u64 time      = utils::currtimeInMilliseconds() - starttime;

            result.score   = score;
            result.is_mate = false;
            result.mate_in = 0;
            result.depth   = 1;
            result.time    = time;
            result.pv      = {};

            searchProgressReportHandler(result);
            searchCompleteReportHander(result);

            return result;
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

        Searcher& Searcher::operator=(const Searcher&) { return *this; }

    }

}
