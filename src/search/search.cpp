#include "search.h"
#include "commons/utils.h"
#include "core/movegen.h"
#include "eval/hce/eval.h"
#include "search/movepicker.h"
#include "search/params.h"
#include "search/timeman.h"

namespace sagittar::search {

    Searcher::Searcher() :
        m_n_threads(1) {
        reset();
    }

    void Searcher::reset() {
        m_workers.clear();
        m_tt.clear();
    }

    void Searcher::resetForSearch() {
        m_workers.clear();
        m_tt.resetForSearch();
    }

    void Searcher::setTranspositionTableSize(const std::size_t size) { m_tt.setSize(size); }

    void Searcher::setThreadCount(const std::size_t n) { m_n_threads = n; }

    SearchResult Searcher::startSearch(const Position&                          pos,
                                       std::span<U64>                           key_history,
                                       SearchInfo                               info,
                                       std::function<void(const SearchResult&)> onProgress,
                                       std::function<void(const SearchResult&)> onComplete) {
        setSearchHardBoundTime(&info, pos);

        m_workers.clear();
        m_workers.reserve(m_n_threads);

        for (size_t i = 0; i < m_n_threads; i++)
        {
            m_workers.emplace_back(std::make_unique<Worker>(key_history, info, m_tt));
        }

        std::vector<std::future<SearchResult>> futures;
        futures.reserve(m_n_threads);

        for (size_t id = 0; id < m_n_threads; ++id)
        {
            futures.emplace_back(std::async(
              std::launch::async, [this, id, &pos, onProgress, onComplete]() -> SearchResult {
                  Worker& w = *m_workers[id];

                  if (id == 0)
                  {
                      // Main thread
                      return w.start(pos, onProgress, onComplete);
                  }
                  else
                  {
                      // Helper threads
                      return w.start(pos, [](const auto&) { }, [](const auto&) { });
                  }
              }));
        }

        SearchResult result = futures[0].get();

        for (size_t id = 1; id < m_n_threads; ++id)
        {
            if (futures[id].valid())
            {
                futures[id].wait();
            }
        }

        m_workers.clear();

        return result;
    }

    SearchResult
    Searcher::startSearch(const Position& pos, std::span<U64> key_history, SearchInfo info) {
        return startSearch(pos, key_history, info, [](auto&) { }, [](auto&) { });
    }

    void Searcher::stopSearch() {
        for (auto& w : m_workers)
        {
            if (w)
            {
                w->stop();
            }
        }
    }

    Searcher::Worker::Worker(std::span<U64>      key_history_ptr,
                             const SearchInfo&   info,
                             TranspositionTable& tt) :
        m_info(info),
        m_tt(tt) {
        m_key_history.reserve(1024);
        m_key_history.clear();
        std::ranges::copy(key_history_ptr, std::back_inserter(m_key_history));
    }

    void Searcher::Worker::checkTimeUp() {
        if (m_info.timeset && (utils::currtimeInMilliseconds() >= m_info.stoptime))
        {
            m_should_stop.store(true, std::memory_order_relaxed);
        }
    }

    bool Searcher::Worker::doMove(Position& pos, const Move& move) {
        m_key_history.push_back(pos.key());
        return pos.doMove(move);
    }

    void Searcher::Worker::doNullMove(Position& pos) {
        m_key_history.push_back(pos.key());
        pos.doNullMove();
    }

    void Searcher::Worker::undoMove() { m_key_history.pop_back(); }

    void Searcher::Worker::undoNullMove() { m_key_history.pop_back(); }

    void Searcher::Worker::updateHistory(const Piece p, const Square to, const I32 bonus) {
        const auto clamped_bonus = std::clamp<I16>(bonus, -k_MAX_HISTORY, k_MAX_HISTORY);
        m_history[p][to] +=
          clamped_bonus - m_history[p][to] * std::abs(clamped_bonus) / k_MAX_HISTORY;
    }

    SearchResult Searcher::Worker::start(const Position&                          pos,
                                         std::function<void(const SearchResult&)> onProgress,
                                         std::function<void(const SearchResult&)> onComplete) {
        SearchResult bestresult{};

        Score alpha = -k_INF;
        Score beta  = k_INF;

        for (Depth currdepth = 1; currdepth <= m_info.depth; currdepth++)
        {
            m_nodes = 0;

#ifndef NDEBUG
            m_instrumentation_store = {};
#endif

            const U64 starttime = utils::currtimeInMilliseconds();
            Score     score     = search<NodeType::ROOT>(pos, currdepth, alpha, beta, 0, true);
            const U64 time      = utils::currtimeInMilliseconds() - starttime;

            if (m_should_stop.load(std::memory_order_relaxed))
            {
                break;
            }

            // Aspiration Windows
            if ((score <= alpha) || (score >= beta))
            {
                // We fell outside the window
                // Try again with a full-width window (and the same depth).
                alpha = -k_INF;
                beta  = k_INF;
                currdepth--;
                continue;
            }
            else
            {
                alpha = score - 50;
                beta  = score + 50;
            }

            SearchResult result{};

            result.score = score;
            if (score > -k_MATE_VALUE && score < -k_MATE_SCORE)
            {
                result.is_mate = true;
                result.mate_in = (-(score + k_MATE_VALUE) / 2 - 1);
            }
            else if (score > k_MATE_SCORE && score < k_MATE_VALUE)
            {
                result.is_mate = true;
                result.mate_in = ((k_MATE_VALUE - score) / 2 + 1);
            }
            else
            {
                result.is_mate = false;
                result.mate_in = 0;
            }
            result.depth    = currdepth;
            result.nodes    = m_nodes;
            result.time     = time;
            result.hashfull = m_tt.hashfull();
            result.bestmove = m_pvmove;
            result.pv       = {result.bestmove};

#ifndef NDEBUG
            result.instrumentation = m_instrumentation_store;
#endif

            onProgress(result);

            bestresult = result;
        }

        onComplete(bestresult);

        return bestresult;
    }

    void Searcher::Worker::stop() { m_should_stop.store(true, std::memory_order_relaxed); }

    template<Searcher::Worker::NodeType nodeType>
    Score Searcher::Worker::search(const Position& pos,
                                   Depth           depth,
                                   Score           alpha,
                                   Score           beta,
                                   const I32       ply,
                                   const bool      do_null) {

        constexpr bool is_root_node    = (nodeType == NodeType::ROOT);
        constexpr bool is_pv_node_type = (nodeType != NodeType::NON_PV);
        const bool     is_pv_node      = ((beta - alpha) > 1) || is_pv_node_type;

        if constexpr (!is_root_node)
        {
            if ((m_nodes & 2047) == 0)
            {
                checkTimeUp();
                if (m_should_stop.load(std::memory_order_relaxed))
                {
                    return 0;
                }
            }

            if (ply >= k_MAX_DEPTH - 1) [[unlikely]]
            {
                return eval::hce::evaluate(pos);
            }

            if ((do_null && pos.isDrawn(m_key_history)) || (pos.halfmoves() >= 100))
            {
                return 0;
            }
        }

        const bool is_in_check = pos.isInCheck();

        if (is_in_check)
        {
            depth++;
        }

        if (depth <= 0)
        {
            return quiescencesearch(pos, alpha, beta, ply);
        }

        const bool is_critical_node = is_pv_node || is_in_check;

        TTData     ttdata;
        const bool tthit = m_tt.probe(&ttdata, pos.key());

        // TT cutoff
        if (!is_pv_node && tthit && ttdata.depth >= depth)
        {
            Score ttscore = ttdata.score;
            if (ttscore < -k_MATE_SCORE)
            {
                ttscore += ply;
            }
            else if (ttscore > k_MATE_SCORE)
            {
                ttscore -= ply;
            }

            if (ttdata.flag == TTFlag::EXACT
                || (ttdata.flag == TTFlag::LOWERBOUND && ttscore >= beta)
                || (ttdata.flag == TTFlag::UPPERBOUND && ttscore <= alpha))
            {
                return ttscore;
            }
        }

        bool do_futility_pruning = false;

        if (!is_critical_node)
        {
            const Score static_eval = eval::hce::evaluate(pos);

            // Reverse Futility Pruning
            if (depth <= 8)
            {
                const Score margin = params::rfpMargin() * depth;
                if (static_eval >= beta + margin)
                {
                    return static_eval;
                }
            }

            // Null Move Pruning
            if (do_null && depth >= 3 && !eval::hce::isEndGame(pos))
            {
                U8 r = 2;
                r += (depth > 7);
                Position pos_copy = pos;
                doNullMove(pos_copy);
                const Score score =
                  -search<NodeType::NON_PV>(pos_copy, depth - r, -beta, -beta + 1, ply, false);
                undoNullMove();
                if (score >= beta)
                {
                    return beta;
                }
            }

            // Futility Pruning Decision
            // clang-format off
            if (depth <= 7
                && alpha < k_WIN_SCORE
                && (((static_eval + ((params::futilityMarginM() * depth) + params::futilityMarginC())) <= alpha)))
            {
                do_futility_pruning = true;
            }
            // clang-format on

            // Internal Iterative Reductions
            if (depth >= 4 && (!tthit || (ttdata.move == k_NULL_MOVE)))
            {
                depth--;
            }
        }

        StackEntry& ss = m_stack[ply];

        Score  best_score = -k_INF;
        Move   best_move_so_far;
        TTFlag ttflag            = TTFlag::UPPERBOUND;
        U32    legal_moves_count = 0;
        U32    moves_searched    = 0;

        const Move ttmove = is_root_node ? m_pvmove : tthit ? ttdata.move : Move{};

        std::array<ExtMove, k_MOVES_MAX> buffer{};
        MovePicker move_picker(buffer.data(), pos, ttmove, m_history, ss.killers[0], ss.killers[1],
                               MovegenType::ALL);

        while (move_picker.hasNext())
        {
            const Move move = move_picker.next();

            Position pos_copy = pos;
            if (!doMove(pos_copy, move))
            {
                undoMove();
                continue;
            }

            const Piece     move_piece      = pos.pieceOn(move.from());
            const PieceType move_piece_type = pieceTypeOf(move_piece);
            const bool      move_is_capture = move.isCapture();

            legal_moves_count++;

            const bool move_is_quite    = !(move_is_capture || move.isPromotion());
            const bool move_gives_check = pos_copy.isInCheck();

            // Move Loop Pruning
            if (moves_searched > 0 && !is_critical_node && move_is_quite && !move_gives_check)
            {
                // Futility Pruning
                if (do_futility_pruning)
                {
                    undoMove();
                    continue;
                }

                // Late Move Pruning
                if (depth <= 8 && move_piece_type != PieceType::PAWN)
                {

#ifndef NDEBUG
                    m_instrumentation_store.lmpEligible++;
#endif

                    const int threshold =
                      (100 * params::lmpBase() + params::lmpDepthScale() * depth * depth) / 200;

                    if (moves_searched >= static_cast<U32>(threshold))
                    {
#ifndef NDEBUG
                        m_instrumentation_store.lmpPruned++;
#endif
                        undoMove();
                        continue;
                    }
                }
            }

            m_nodes++;

            Score score = -k_INF;

            if (!is_pv_node || moves_searched > 0)
            {
                const bool can_reduce = (depth >= 3) && (moves_searched >= 4) && (!is_critical_node)
                                     && (!move_gives_check)
                                     && (move_picker.phase() != MovePickerPhase::KILLERS);

                if (can_reduce)
                {
                    const U8 r =
                      move_is_quite
                        ? params::g_lmr_r_table_quiet[std::min(moves_searched, 64U)][(int) depth]
                        : params::g_lmr_r_table_tactical[std::min(moves_searched, 64U)]
                                                        [(int) depth];

                    score = -search<NodeType::NON_PV>(pos_copy, depth - r, -alpha - 1, -alpha,
                                                      ply + 1, do_null);
                }

                if (!can_reduce || score > alpha)
                {
                    score = -search<NodeType::NON_PV>(pos_copy, depth - 1, -alpha - 1, -alpha,
                                                      ply + 1, do_null);
                }
            }

            if (is_pv_node && ((moves_searched == 0) || (score > alpha && score < beta)))
            {
                score = -search<NodeType::PV>(pos_copy, depth - 1, -beta, -alpha, ply + 1, do_null);
            }

            moves_searched++;

            undoMove();

            if (m_should_stop.load(std::memory_order_relaxed))
            {
                return 0;
            }

            // Fail-soft
            if (score > best_score)
            {
                best_score = score;
                if (best_score > alpha)
                {
                    alpha            = score;
                    best_move_so_far = move;
                    ttflag           = TTFlag::EXACT;
                    if (best_score >= beta)
                    {
                        if (!move_is_capture)
                        {
                            // Killer Heuristic
                            ss.killers[1] = ss.killers[0];
                            ss.killers[0] = move;

                            // History Heuristic
                            updateHistory(move_piece, move.to(), depth * depth);
                        }
                        ttflag = TTFlag::LOWERBOUND;
                        break;
                    }
                }
            }
        }

        if (legal_moves_count == 0)
        {
            return is_in_check ? (-k_MATE_VALUE + ply) : 0;
        }

        if (!m_should_stop.load(std::memory_order_relaxed))
        {
            m_tt.store(pos.key(), ply, depth, ttflag, best_score, best_move_so_far);

            if constexpr (is_root_node)
            {
                m_pvmove = best_move_so_far;
            }
        }

        return best_score;
    }

    Score Searcher::Worker::quiescencesearch(const Position& pos,
                                             Score           alpha,
                                             Score           beta,
                                             const I32       ply) {
        const Score alpha_orig = alpha;

        if ((m_nodes & 2047) == 0)
        {
            checkTimeUp();
            if (ply > 0 && m_should_stop.load(std::memory_order_relaxed))
            {
                return 0;
            }
        }

        const bool is_in_check = pos.isInCheck();

        if (ply >= k_MAX_DEPTH - 1)
        {
            return is_in_check ? 0 : eval::hce::evaluate(pos);
        }

        TTData     ttdata;
        const bool tthit = m_tt.probe(&ttdata, pos.key());

        // TT cutoff
        if (tthit)
        {
            Score ttscore = ttdata.score;
            if (ttscore < -k_MATE_SCORE)
            {
                ttscore += ply;
            }
            else if (ttscore > k_MATE_SCORE)
            {
                ttscore -= ply;
            }

            if (ttdata.flag == TTFlag::EXACT
                || (ttdata.flag == TTFlag::LOWERBOUND && ttscore >= beta)
                || (ttdata.flag == TTFlag::UPPERBOUND && ttscore <= alpha))
            {
                return ttscore;
            }
        }

        Score eval;

        if (is_in_check)
        {
            eval = -k_MATE_VALUE + ply;
        }
        else
        {
            eval = eval::hce::evaluate(pos);
            if (eval >= beta)
            {
                return beta;
            }
            if (alpha < eval)
            {
                alpha = eval;
            }
        }

        Score best_score        = eval;
        Move  best_move_so_far  = k_NULL_MOVE;
        U32   legal_moves_count = 0;

        const Move        ttmove = tthit ? ttdata.move : k_NULL_MOVE;
        const MovegenType movegen_type =
          is_in_check ? MovegenType::CHECK_EVASIONS : MovegenType::CAPTURES;

        StackEntry& ss = m_stack[ply];

        std::array<ExtMove, k_MOVES_MAX> buffer{};
        MovePicker move_picker(buffer.data(), pos, ttmove, m_history, ss.killers[0], ss.killers[1],
                               movegen_type);

        while (move_picker.hasNext())
        {
            const Move move = move_picker.next();

            Position pos_copy = pos;
            if (!doMove(pos_copy, move))
            {
                undoMove();
                continue;
            }

            legal_moves_count++;
            m_nodes++;

            const Score score = -quiescencesearch(pos_copy, -beta, -alpha, ply + 1);

            undoMove();

            if (m_should_stop.load(std::memory_order_relaxed))
            {
                return 0;
            }

            best_score = std::max(best_score, score);

            if (score > alpha)
            {
                alpha            = score;
                best_move_so_far = move;
                if (score >= beta)
                {
                    m_tt.store(pos.key(), ply, 0, TTFlag::LOWERBOUND, beta, best_move_so_far);
                    return beta;
                }
            }
        }

        if ((legal_moves_count == 0) && is_in_check)
        {
            return -k_MATE_VALUE + ply;
        }

        if (!m_should_stop.load(std::memory_order_relaxed))
        {
            const TTFlag flag = (best_score <= alpha_orig) ? TTFlag::UPPERBOUND
                              : (best_score >= beta)       ? TTFlag::LOWERBOUND
                                                           : TTFlag::EXACT;
            m_tt.store(pos.key(), ply, 0, flag, best_score, best_move_so_far);
        }

        return best_score;
    }

}
