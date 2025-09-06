#include "search.h"
#include "core/move.h"
#include "core/movegen.h"
#include "core/utils.h"
#include "eval/hce/eval.h"

namespace sagittar {

    namespace search {

        static constexpr int INF        = 32000;
        static constexpr int MATE_SCORE = 30000;
        static constexpr int DEPTH_MAX  = 64;

        Searcher::Searcher() { reset(); }

        void Searcher::reset() { stopped.store(false, std::memory_order_relaxed); }

        void Searcher::reset_for_search() {}

        void Searcher::stop() { stopped.store(true, std::memory_order_relaxed); }

        SearchResult Searcher::start(Position const*                          pos,
                                     SearchInfo                               info,
                                     std::function<void(const SearchResult&)> progress_handler,
                                     std::function<void(const SearchResult&)> complete_hander) {
            stopped.store(false, std::memory_order_relaxed);
            return search(pos, info, progress_handler, complete_hander);
        }

        SearchResult Searcher::start(Position const* pos, SearchInfo info) {
            return start(pos, info, [](auto&) {}, [](auto&) {});
        }

        void Searcher::check_timeup(const SearchInfo& info) {
            if (info.timeset && (currtime_ms() >= info.stoptime))
            {
                stopped.store(true, std::memory_order_relaxed);
            }
        }

        SearchResult Searcher::search(Position const*                          pos,
                                      const SearchInfo&                        info,
                                      std::function<void(const SearchResult&)> progress_handler,
                                      std::function<void(const SearchResult&)> complete_hander) {
            SearchResult result{};
            const auto   starttime = currtime_ms();
            const Score  score     = search_root(pos, 4, 0, info, &result);
            const auto   time      = currtime_ms() - starttime;

            result.score         = score;
            const auto score_abs = abs(score);
            if (score_abs >= (MATE_SCORE - DEPTH_MAX))
            {
                result.is_mate          = true;
                const int moves_to_mate = ((MATE_SCORE - score_abs) + 1) / 2;
                result.mate_in          = (score > 0) ? moves_to_mate : -moves_to_mate;
            }
            result.depth = info.depth;
            result.time  = time;

            progress_handler(result);
            complete_hander(result);

            return result;
        }

        Score Searcher::search_root(Position const*   pos,
                                    int               depth,
                                    const int         ply,
                                    const SearchInfo& info,
                                    SearchResult*     result) {
            if (depth <= 0)
            {
                return eval::hce::eval(pos);
            }

            Score max               = -INF;
            Move  bestmovesofar     = NULL_MOVE;
            int   legal_moves_count = 0;

            MoveList moves_list = {};
            movegen_generate_pseudolegal_moves<MovegenType::MOVEGEN_ALL>(pos, &moves_list);

            for (auto [move, move_score] : moves_list)
            {
                Position pos_dup = *pos;
                if (!pos_dup.do_move(move))
                {
                    continue;
                }
                legal_moves_count++;
                result->nodes++;
                const Score score = -search_negamax(&pos_dup, depth - 1, ply + 1, info, result);
                if (stopped.load(std::memory_order_relaxed))
                {
                    return 0;
                }
                if (score > max)
                {
                    max           = score;
                    bestmovesofar = move;
                }
            }

            result->bestmove = bestmovesofar;

            if (legal_moves_count == 0)
            {
                return pos->is_in_check() ? ply - MATE_SCORE : 0;
            }

            return max;
        }

        Score Searcher::search_negamax(Position const*   pos,
                                       int               depth,
                                       const int         ply,
                                       const SearchInfo& info,
                                       SearchResult*     result) {
            if ((result->nodes & 2047) == 0)
            {
                check_timeup(info);
                if (stopped.load(std::memory_order_relaxed))
                {
                    return 0;
                }
            }

            if (depth <= 0)
            {
                return eval::hce::eval(pos);
            }

            Score max               = -INF;
            int   legal_moves_count = 0;

            MoveList moves_list = {};
            movegen_generate_pseudolegal_moves<MovegenType::MOVEGEN_ALL>(pos, &moves_list);

            for (auto [move, move_score] : moves_list)
            {
                Position pos_dup = *pos;
                if (!pos_dup.do_move(move))
                {
                    continue;
                }
                legal_moves_count++;
                result->nodes++;
                const Score score = -search_negamax(&pos_dup, depth - 1, ply + 1, info, result);
                if (stopped.load(std::memory_order_relaxed))
                {
                    return 0;
                }
                if (score > max)
                {
                    max = score;
                }
            }

            if (legal_moves_count == 0)
            {
                return pos->is_in_check() ? ply - MATE_SCORE : 0;
            }

            return max;
        }

    }

}
