#include "search.h"
#include "core/move.h"
#include "core/movegen.h"
#include "core/utils.h"
#include "eval/hce/eval.h"
#include "search/movepicker.h"
#include "search/timeman.h"

namespace sagittar {

    namespace search {

        Searcher::ThreadData::ThreadData() {
            hash_history.reserve(1024);
            hash_history.clear();
        }

        std::pair<bool, Position> Searcher::ThreadData::do_move(const Position& pos,
                                                                const Move      move) {
            Position   new_pos  = pos;
            const bool is_valid = pos.do_move(move, &new_pos);
            hash_history.push_back(new_pos.hash);
            return std::make_pair(is_valid, new_pos);
        }

        void Searcher::ThreadData::undo_move() { hash_history.pop_back(); }

        Searcher::Searcher() { reset(); }

        void Searcher::reset() {
            stopped.store(false, std::memory_order_relaxed);
            tt.clear();
            pv_move = NULL_MOVE;
        }

        void Searcher::reset_for_search() {}

        void Searcher::set_tt_size(const size_t mb) { tt.resize(mb); }

        void Searcher::stop() { stopped.store(true, std::memory_order_relaxed); }

        SearchResult Searcher::start(const Position&                          pos,
                                     std::span<uint64_t>                      hash_history,
                                     SearchInfo                               info,
                                     std::function<void(const SearchResult&)> progress_handler,
                                     std::function<void(const SearchResult&)> complete_hander) {
            stopped.store(false, std::memory_order_relaxed);
            pv_move               = NULL_MOVE;
            History    hist_table = {};
            ThreadData thread{};
            std::ranges::copy(hash_history, std::back_inserter(thread.hash_history));
            if (pos.black_to_play)
                set_hardbound_time<BLACK>(&info);
            else
                set_hardbound_time<WHITE>(&info);
            return search_pos(pos, &hist_table, thread, info, progress_handler, complete_hander);
        }

        SearchResult
        Searcher::start(const Position& pos, std::span<uint64_t> hash_history, SearchInfo info) {
            return start(pos, hash_history, info, [](auto&) {}, [](auto&) {});
        }

        void Searcher::check_timeup(const SearchInfo& info) {
            if (info.timeset && (currtime_ms() >= info.stoptime))
            {
                stopped.store(true, std::memory_order_relaxed);
            }
        }

        SearchResult
        Searcher::search_pos(const Position&                          pos,
                             History* const                           hist_table,
                             ThreadData&                              thread,
                             const SearchInfo&                        info,
                             std::function<void(const SearchResult&)> progress_handler,
                             std::function<void(const SearchResult&)> complete_hander) {
            SearchResult best_result{};

            for (int currdepth = 1; currdepth <= info.depth; currdepth++)
            {
                SearchResult result{};
                const auto   starttime = currtime_ms();
                const Score  score     = search<Searcher::NodeType::ROOT>(
                  pos, currdepth, -INF, INF, 0, hist_table, thread, info, &result);
                const auto time = currtime_ms() - starttime;

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

        template<Searcher::NodeType nodeType>
        Score Searcher::search(const Position&   pos,
                               int               depth,
                               Score             alpha,
                               Score             beta,
                               const int         ply,
                               History* const    hist_table,
                               ThreadData&       thread,
                               const SearchInfo& info,
                               SearchResult*     result) {

            constexpr bool is_root_node = (nodeType == Searcher::NodeType::ROOT);

            if constexpr (!is_root_node)
            {
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

                if (pos.is_repeated(thread.hash_history) || pos.half_moves >= 100)
                {
                    return 0;
                }
            }

            const bool is_in_check = pos.is_in_check();

            depth += is_in_check;

            if (depth <= 0)
            {
                return search_quiescence(pos, alpha, beta, ply, hist_table, thread, info, result);
            }

            TTData<Score> ttdata{};
            const bool    tthit = tt.probe(&ttdata, pos.hash, ply);

            if constexpr (!is_root_node)
            {
                if (tthit && ttdata.depth >= depth)
                {
                    if (ttdata.flag == TT_FLAG_EXACT
                        || (ttdata.flag == TT_FLAG_LOWERBOUND && ttdata.value >= beta)
                        || (ttdata.flag == TT_FLAG_UPPERBOUND && ttdata.value <= alpha))
                    {
                        return ttdata.value;
                    }
                }
            }

            Score  best_score        = -INF;
            Move   best_move         = NULL_MOVE;
            TTFlag flag              = TT_FLAG_UPPERBOUND;
            int    legal_moves_count = 0;

            MoveList moves_list = {};
            movegen_generate_pseudolegal_moves<MovegenType::MOVEGEN_ALL>(pos, &moves_list);

            Move tt_move;
            if constexpr (nodeType == Searcher::NodeType::ROOT)
            {
                tt_move = pv_move;
            }
            else
            {
                tt_move = ttdata.move;
            }

            MovePicker move_picker(&moves_list, pos, tt_move, hist_table);

            while (move_picker.has_next())
            {
                const auto [move, move_score] = move_picker.next();

                const auto [is_valid, new_pos] = thread.do_move(pos, move);
                if (!is_valid)
                {
                    thread.undo_move();
                    continue;
                }
                legal_moves_count++;
                result->nodes++;
                const Score score = -search<Searcher::NodeType::NON_ROOT>(
                  new_pos, depth - 1, -beta, -alpha, ply + 1, hist_table, thread, info, result);
                thread.undo_move();
                if (stopped.load(std::memory_order_relaxed))
                {
                    return 0;
                }
                // Fail-soft
                best_score = std::max(best_score, score);
                if (best_score > alpha)
                {
                    best_move = move;
                    if (best_score >= beta)
                    {
                        if (!MOVE_IS_CAPTURE(move))
                        {
                            const Piece p = pos.board.pieces[MOVE_FROM(move)];
                            hist_table->at(p).at(MOVE_TO(move)) += (depth * depth);
                        }

                        flag = TT_FLAG_LOWERBOUND;
                        break;
                    }
                    alpha = best_score;
                    flag  = TT_FLAG_EXACT;
                }
            }

            if (legal_moves_count == 0)
            {
                return is_in_check * (ply - MATE_SCORE);
            }

            if (!stopped.load(std::memory_order_relaxed))
            {
                if constexpr (is_root_node)
                {
                    pv_move          = best_move;
                    result->bestmove = best_move;
                }
                tt.store(pos.hash, depth, ply, flag, best_score, best_move);
            }

            return best_score;
        }

        Score Searcher::search_quiescence(const Position&   pos,
                                          Score             alpha,
                                          Score             beta,
                                          const int         ply,
                                          History* const    hist_table,
                                          ThreadData&       thread,
                                          const SearchInfo& info,
                                          SearchResult*     result) {

            if ((result->nodes & 2047) == 0)
            {
                check_timeup(info);
                if (stopped.load(std::memory_order_relaxed) && ply > 0)
                {
                    return 0;
                }
            }

            if (ply >= DEPTH_MAX - 1)
            {
                return eval::hce::eval(pos);
            }

            const Score stand_pat = eval::hce::eval(pos);
            if (stand_pat >= beta)
            {
                return beta;
            }
            alpha = std::max(alpha, stand_pat);

            TTData<Score> ttdata{};
            tt.probe(&ttdata, pos.hash, ply);

            MoveList moves_list = {};
            movegen_generate_pseudolegal_moves<MovegenType::MOVEGEN_CAPTURES>(pos, &moves_list);

            MovePicker move_picker(&moves_list, pos, ttdata.move, hist_table);

            while (move_picker.has_next())
            {
                const auto [move, move_score] = move_picker.next();

                const auto [is_valid, new_pos] = thread.do_move(pos, move);
                if (!is_valid)
                {
                    thread.undo_move();
                    continue;
                }
                result->nodes++;
                const Score score = -search_quiescence(new_pos, -beta, -alpha, ply + 1, hist_table,
                                                       thread, info, result);
                thread.undo_move();
                if (stopped.load(std::memory_order_relaxed))
                {
                    return 0;
                }
                if (score >= beta)
                {
                    return beta;
                }
                alpha = std::max(alpha, score);
            }

            return alpha;
        }

    }

}
