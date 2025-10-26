#include "search.h"
#include "containers.h"
#include "eval.h"
#include "movegen.h"
#include "movepicker.h"
#include "params.h"
#include "timeman.h"
#include "utils.h"

namespace sagittar {

    namespace search {

        SearcherData::SearcherData() { reset(); }

        void SearcherData::reset() {
            for (u8 p = Piece::NO_PIECE; p <= Piece::BLACK_KING; p++)
            {
                for (u8 sq = Square::A1; sq <= Square::H8; sq++)
                {
                    history[p][sq] = 0;
                }
            }
            for (u8 i = 0; i < MAX_DEPTH; i++)
            {
                killer_moves[0][i] = move::Move();
                killer_moves[1][i] = move::Move();
            }
        }

        Searcher::ThreadData::ThreadData() :
            nodes(0) {
            key_history.reserve(1024);
            key_history.shrink_to_fit();
            key_history.clear();
        }

        core::DoMoveResult Searcher::ThreadData::doMove(core::Position&  pos,
                                                        const move::Move move) {
            key_history.push_back(pos.getHash());
            return pos.doMove(move);
        }

        void Searcher::ThreadData::doNullMove(core::Position& pos) {
            key_history.push_back(pos.getHash());
            pos.doNullMove();
        }

        void Searcher::ThreadData::undoMove() { key_history.pop_back(); }

        void Searcher::ThreadData::undoNullMove() { key_history.pop_back(); }

        /*
        * Searcher
        */

        Searcher::Searcher() { reset(); }

        void Searcher::reset() {
            stop.store(false, std::memory_order_relaxed);
            tt.clear();
            data.reset();
        }

        void Searcher::resetForSearch() { tt.resetForSearch(); }

        void Searcher::setTranspositionTableSize(const std::size_t size) { tt.setSize(size); }

        SearchResult
        Searcher::startSearch(const core::Position&                    pos,
                              std::span<u64>                           key_history,
                              SearchInfo                               info,
                              std::function<void(const SearchResult&)> searchProgressReportHandler,
                              std::function<void(const SearchResult&)> searchCompleteReportHander) {
            stop.store(false, std::memory_order_relaxed);
            timeman::setSearchHardBoundTime(&info, pos);

            ThreadData thread{};
            std::ranges::copy(key_history, std::back_inserter(thread.key_history));

            return searchIteratively(pos, thread, info, searchProgressReportHandler,
                                     searchCompleteReportHander);
        }

        SearchResult Searcher::startSearch(const core::Position& pos,
                                           std::span<u64>        key_history,
                                           SearchInfo            info) {
            return startSearch(pos, key_history, info, [](auto&) {}, [](auto&) {});
        }

        void Searcher::stopSearch() { stop.store(true, std::memory_order_relaxed); }

        void Searcher::shouldStopSearchNow(const SearchInfo& info) {
            if (info.timeset && (utils::currtimeInMilliseconds() >= info.stoptime))
            {
                stop.store(true, std::memory_order_relaxed);
            }
        }

        SearchResult Searcher::searchIteratively(
          const core::Position&                    pos,
          ThreadData&                              thread,
          const SearchInfo&                        info,
          std::function<void(const SearchResult&)> searchProgressReportHandler,
          std::function<void(const SearchResult&)> searchCompleteReportHander) {
            SearchResult bestresult{};

            Score alpha = -INF;
            Score beta  = INF;

            for (Depth currdepth = 1; currdepth <= info.depth; currdepth++)
            {
                thread.nodes = 0;

                const u64 starttime = utils::currtimeInMilliseconds();
                Score     score =
                  search<NodeType::ROOT>(pos, currdepth, alpha, beta, 0, thread, info, true);
                const u64 time = utils::currtimeInMilliseconds() - starttime;

                if (stop.load(std::memory_order_relaxed))
                {
                    break;
                }

                // Aspiration Windows
                if ((score <= alpha) || (score >= beta))
                {
                    // We fell outside the window
                    // Try again with a full-width window (and the same depth).
                    alpha = -INF;
                    beta  = INF;
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
                result.depth    = currdepth;
                result.nodes    = thread.nodes;
                result.time     = time;
                result.hashfull = tt.hashfull();
                result.bestmove = thread.pvmove;
                result.pv       = {result.bestmove};

                searchProgressReportHandler(result);

                bestresult = result;
            }

            searchCompleteReportHander(bestresult);

            return bestresult;
        }

        template<NodeType nodeType>
        Score Searcher::search(const core::Position& pos,
                               Depth                 depth,
                               Score                 alpha,
                               Score                 beta,
                               const i32             ply,
                               ThreadData&           thread,
                               const SearchInfo&     info,
                               const bool            do_null) {

            constexpr bool is_root_node    = (nodeType == NodeType::ROOT);
            constexpr bool is_pv_node_type = (nodeType != NodeType::NON_PV);
            const bool     is_pv_node      = ((beta - alpha) > 1) || is_pv_node_type;

            if constexpr (!is_root_node)
            {
                if ((thread.nodes & 2047) == 0)
                {
                    shouldStopSearchNow(info);
                    if (stop.load(std::memory_order_relaxed))
                    {
                        return 0;
                    }
                }

                if (ply >= MAX_DEPTH - 1) [[unlikely]]
                {
                    return eval::evaluate(pos);
                }

                if ((do_null && pos.hasPositionRepeated(thread.key_history))
                    || (pos.getHalfmoveClock() >= 100))
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
                return quiescencesearch(pos, alpha, beta, ply, thread, info);
            }

            const bool is_critical_node = is_pv_node || is_in_check;

            tt::TTData ttdata;
            const bool tthit = tt.probe(&ttdata, pos.getHash());

            // TT cutoff
            if (!is_pv_node && tthit && ttdata.depth >= depth)
            {
                Score ttscore = ttdata.score;
                if (ttscore < -MATE_SCORE)
                {
                    ttscore += ply;
                }
                else if (ttscore > MATE_SCORE)
                {
                    ttscore -= ply;
                }

                if (ttdata.flag == tt::TTFlag::EXACT
                    || (ttdata.flag == tt::TTFlag::LOWERBOUND && ttscore >= beta)
                    || (ttdata.flag == tt::TTFlag::UPPERBOUND && ttscore <= alpha))
                {
                    return ttscore;
                }
            }

            bool do_futility_pruning = false;

            if (!is_critical_node)
            {
                const Score static_eval = eval::evaluate(pos);

                // Reverse Futility Pruning
                if (depth <= 3)
                {
                    const Score margin = params::rfp_margin * depth;
                    if (static_eval >= beta + margin)
                    {
                        return static_eval;
                    }
                }

                // Null Move Pruning
                if (do_null && depth >= 3 && !eval::isEndGame(pos))
                {
                    const u8       r        = 2;
                    core::Position pos_copy = pos;
                    thread.doNullMove(pos_copy);
                    const Score score = -search<NodeType::NON_PV>(
                      pos_copy, depth - r, -beta, -beta + 1, ply, thread, info, false);
                    thread.undoNullMove();
                    if (score >= beta)
                    {
                        return beta;
                    }
                }

                // Futility Pruning Decision
                // clang-format off
                if (depth <= 3
                    && alpha < WIN_SCORE
                    && ((static_eval + params::futility_margin[(int) depth]) <= alpha))
                {
                    do_futility_pruning = true;
                }
                // clang-format on

                // Internal Iterative Reductions
                if (depth >= 4 && (!tthit || (ttdata.move == move::Move())))
                {
                    depth--;
                }
            }

            Score      best_score = -INF;
            move::Move best_move_so_far;
            tt::TTFlag ttflag            = tt::TTFlag::UPPERBOUND;
            u32        legal_moves_count = 0;
            u32        moves_searched    = 0;

            const move::Move ttmove = is_root_node ? thread.pvmove
                                    : tthit        ? ttdata.move
                                                   : move::Move{};

            std::array<move::ExtMove, MOVES_MAX>  buffer{};
            MovePicker<movegen::MovegenType::ALL> move_picker(buffer.data(), pos, ttmove, data,
                                                              ply);
            const auto                            n_moves = move_picker.size();

            while (move_picker.hasNext())
            {
                const move::Move move = move_picker.next();

                core::Position           pos_copy       = pos;
                const core::DoMoveResult do_move_result = thread.doMove(pos_copy, move);
                if (do_move_result == core::DoMoveResult::ILLEGAL)
                {
                    thread.undoMove();
                    continue;
                }

                const Piece     move_piece      = pos.getPiece(move.from());
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
                        thread.undoMove();
                        continue;
                    }

                    // Late Move Pruning
                    if (depth <= 2 && move_piece_type != PieceType::PAWN)
                    {
                        const u32 LMP_MOVE_CUTOFF =
                          n_moves * (1 - (params::lmp_treshold_pct - (0.1 * depth)));
                        if (moves_searched >= LMP_MOVE_CUTOFF)
                        {
                            thread.undoMove();
                            continue;
                        }
                    }
                }

                thread.nodes++;

                Score score = -INF;

                if (!is_pv_node || moves_searched > 0)
                {
                    const bool can_reduce = (depth >= 3) && (moves_searched >= 4)
                                         && (!is_critical_node) && (!move_gives_check)
                                         && (move_picker.phase() != MovePickerPhase::KILLERS);

                    if (can_reduce)
                    {
                        const u8 r =
                          move_is_quite
                            ? params::lmr_r_table_quiet[std::min(moves_searched, 64U)][(int) depth]
                            : params::lmr_r_table_tactical[std::min(moves_searched, 64U)]
                                                          [(int) depth];

                        score = -search<NodeType::NON_PV>(pos_copy, depth - r, -alpha - 1, -alpha,
                                                          ply + 1, thread, info, do_null);
                    }

                    if (!can_reduce || score > alpha)
                    {
                        score = -search<NodeType::NON_PV>(pos_copy, depth - 1, -alpha - 1, -alpha,
                                                          ply + 1, thread, info, do_null);
                    }
                }

                if (is_pv_node && ((moves_searched == 0) || (score > alpha && score < beta)))
                {
                    score = -search<NodeType::PV>(pos_copy, depth - 1, -beta, -alpha, ply + 1,
                                                  thread, info, do_null);
                }

                moves_searched++;

                thread.undoMove();

                if (stop.load(std::memory_order_relaxed))
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
                        ttflag           = tt::TTFlag::EXACT;
                        if (best_score >= beta)
                        {
                            if (!move_is_capture)
                            {
                                // Killer Heuristic
                                data.killer_moves[1][ply] = data.killer_moves[0][ply];
                                data.killer_moves[0][ply] = move;

                                // History Heuristic
                                data.history[move_piece][move.to()] += depth;
                            }
                            ttflag = tt::TTFlag::LOWERBOUND;
                            break;
                        }
                    }
                }
            }

            if (legal_moves_count == 0)
            {
                return is_in_check ? (-MATE_VALUE + ply) : 0;
            }

            if (!stop.load(std::memory_order_relaxed))
            {
                tt.store(pos.getHash(), ply, depth, ttflag, best_score, best_move_so_far);

                if constexpr (is_root_node)
                {
                    thread.pvmove = best_move_so_far;
                }
            }

            return best_score;
        }

        Score Searcher::quiescencesearch(const core::Position& pos,
                                         Score                 alpha,
                                         Score                 beta,
                                         const i32             ply,
                                         ThreadData&           thread,
                                         const SearchInfo&     info) {
            if ((thread.nodes & 2047) == 0)
            {
                shouldStopSearchNow(info);
                if (ply > 0 && stop.load(std::memory_order_relaxed))
                {
                    return 0;
                }
            }

            if (ply >= MAX_DEPTH - 1)
            {
                return eval::evaluate(pos);
            }

            const Score stand_pat = eval::evaluate(pos);
            if (stand_pat >= beta)
            {
                return beta;
            }
            if (alpha < stand_pat)
            {
                alpha = stand_pat;
            }

            tt::TTData       ttdata;
            const bool       tthit  = tt.probe(&ttdata, pos.getHash());
            const move::Move ttmove = tthit ? ttdata.move : move::Move();

            std::array<move::ExtMove, MOVES_MAX>       buffer{};
            MovePicker<movegen::MovegenType::CAPTURES> move_picker(buffer.data(), pos, ttmove, data,
                                                                   ply);

            while (move_picker.hasNext())
            {
                const move::Move move = move_picker.next();

                core::Position           pos_copy       = pos;
                const core::DoMoveResult do_move_result = thread.doMove(pos_copy, move);

                if (do_move_result == core::DoMoveResult::ILLEGAL)
                {
                    thread.undoMove();
                    continue;
                }

                thread.nodes++;

                const Score score =
                  -quiescencesearch(pos_copy, -beta, -alpha, ply + 1, thread, info);

                thread.undoMove();

                if (stop.load(std::memory_order_relaxed))
                {
                    return 0;
                }

                if (score > alpha)
                {
                    alpha = score;
                    if (score >= beta)
                    {
                        return beta;
                    }
                }
            }

            return alpha;
        }

    }
}
