#include "search.h"
#include "core/move.h"
#include "core/movegen.h"
#include "core/utils.h"

namespace sagittar {

    namespace search {

        static constexpr int INF        = 32000;
        static constexpr int MATE_SCORE = 30000;
        static constexpr int DEPTH_MAX  = 64;

        Searcher::Searcher() { reset(); }

        void Searcher::reset() { stopped.store(false, std::memory_order_relaxed); }

        void Searcher::reset_for_search() {}

        void Searcher::stop() { stopped.store(true, std::memory_order_relaxed); }

        SearchResult Searcher::start(core::Position const*                    pos,
                                     SearchInfo                               info,
                                     std::function<void(const SearchResult&)> progress_handler,
                                     std::function<void(const SearchResult&)> complete_hander) {
            stopped.store(false, std::memory_order_relaxed);
            return search(pos, info, progress_handler, complete_hander);
        }

        SearchResult Searcher::start(core::Position const* pos, SearchInfo info) {
            return start(pos, info, [](auto&) {}, [](auto&) {});
        }

        void Searcher::check_timeup(const SearchInfo& info) {
            if (info.timeset && (core::currtime_ms() >= info.stoptime))
            {
                stopped.store(true, std::memory_order_relaxed);
            }
        }

        SearchResult Searcher::search(core::Position const*                    pos,
                                      SearchInfo                               info,
                                      std::function<void(const SearchResult&)> progress_handler,
                                      std::function<void(const SearchResult&)> complete_hander) {
            SearchResult      result;
            const core::Score score = search_random(pos, info, &result);
            result.score            = score;

            if (score == MATE_SCORE)
            {
                result.is_mate = true;
                result.mate_in = 0;
            }

            result.depth = 1;
            result.time  = 0;

            progress_handler(result);
            complete_hander(result);

            return result;
        }

        core::Score Searcher::search_random(core::Position const* pos,
                                            const SearchInfo      info,
                                            SearchResult*         result) {
            core::MoveList moves_list = {};
            core::movegen_generate_pseudolegal_moves<core::MovegenType::MOVEGEN_ALL>(pos,
                                                                                     &moves_list);
            for (auto [move, score] : moves_list)
            {
                core::Position pos_dup = *pos;
                if (pos_dup.do_move(move))
                {
                    result->bestmove = move;
                    result->nodes    = 1;
                    return 0;
                }
            }

            result->nodes = 1;

            return pos->is_in_check() ? MATE_SCORE : 0;
        }

    }

}
