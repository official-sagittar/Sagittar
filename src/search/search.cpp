#include "search.h"
#include "core/move.h"
#include "core/movegen.h"
#include "core/utils.h"
#include "eval/hce/eval.h"
#include "search/timeman.h"

namespace sagittar {

    namespace search {

        static constexpr int INF        = 32000;
        static constexpr int MATE_SCORE = 30000;

        Searcher::Searcher() { reset(); }

        void Searcher::reset() { stopped.store(false, std::memory_order_relaxed); }

        void Searcher::reset_for_search() {}

        void Searcher::stop() { stopped.store(true, std::memory_order_relaxed); }

        SearchResult Searcher::start(Position const*                          pos,
                                     SearchInfo                               info,
                                     std::function<void(const SearchResult&)> progress_handler,
                                     std::function<void(const SearchResult&)> complete_hander) {
            stopped.store(false, std::memory_order_relaxed);
            if (pos->black_to_play)
                set_hardbound_time<BLACK>(&info);
            else
                set_hardbound_time<WHITE>(&info);
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
            SearchResult best_result{};

            for (int currdepth = 1; currdepth <= info.depth; currdepth++)
            {
                SearchResult result{};
                const auto   starttime = currtime_ms();
                const Score  score     = search_root(pos, currdepth, -INF, INF, info, &result);
                const auto   time      = currtime_ms() - starttime;

                if (stopped.load(std::memory_order_relaxed))
                {
                    break;
                }

                best_result = result;

                result.score         = score;
                const auto score_abs = abs(score);
                if (score_abs >= (MATE_SCORE - DEPTH_MAX))
                {
                    result.is_mate          = true;
                    const int moves_to_mate = ((MATE_SCORE - score_abs) + 1) / 2;
                    result.mate_in          = (score > 0) ? moves_to_mate : -moves_to_mate;
                }
                result.depth = currdepth;
                result.time  = time;

                progress_handler(result);
            }

            complete_hander(best_result);

            return best_result;
        }

        Score Searcher::search_root(Position const*   pos,
                                    int               depth,
                                    Score             alpha,
                                    Score             beta,
                                    const SearchInfo& info,
                                    SearchResult*     result) {

            const bool is_in_check = pos->is_in_check();

            depth += is_in_check;

            if (depth <= 0)
            {
                return eval::hce::eval(pos);
            }

            Score best_score        = -INF;
            Move  best_move         = NULL_MOVE;
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
                const Score score =
                  -search_alphabeta(&pos_dup, depth - 1, -beta, -alpha, 1, info, result);
                if (stopped.load(std::memory_order_relaxed))
                {
                    return 0;
                }
                // Fail-soft
                best_score = std::max(best_score, score);
                if (best_score > alpha)
                {
                    alpha     = best_score;
                    best_move = move;
                    if (best_score >= beta)
                    {
                        return beta;
                    }
                }
            }

            result->bestmove = best_move;

            if (legal_moves_count == 0)
            {
                return is_in_check * (-MATE_SCORE);
            }

            return best_score;
        }

        Score Searcher::search_alphabeta(Position const*   pos,
                                         int               depth,
                                         Score             alpha,
                                         Score             beta,
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

            if (ply >= DEPTH_MAX - 1) [[unlikely]]
            {
                return eval::hce::eval(pos);
            }

            const bool is_in_check = pos->is_in_check();

            depth += is_in_check;

            if (depth <= 0)
            {
                return eval::hce::eval(pos);
            }

            Score best_score        = -INF;
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
                const Score score =
                  -search_alphabeta(&pos_dup, depth - 1, -beta, -alpha, ply + 1, info, result);
                if (stopped.load(std::memory_order_relaxed))
                {
                    return 0;
                }
                // Fail-soft
                best_score = std::max(best_score, score);
                if (best_score > alpha)
                {
                    alpha = best_score;
                    if (best_score >= beta)
                    {
                        return beta;
                    }
                }
            }

            if (legal_moves_count == 0)
            {
                return is_in_check * (ply - MATE_SCORE);
            }

            return best_score;
        }

    }

}
