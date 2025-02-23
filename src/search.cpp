#include "search.h"
#include "containers.h"
#include "eval.h"
#include "movegen.h"
#include "movepicker.h"
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
        }

        /*
        * Searcher
        */

        Searcher::Searcher() { stop.store(false, std::memory_order_relaxed); }

        void Searcher::setParams(const parameters::ParameterStore& params) {
            searchParams.RFP_DEPTH_MAX = params.get<int>("RFP_DEPTH_MAX", 3);
            searchParams.RFP_MARGIN    = params.get<int>("RFP_MARGIN", 150);
        }

        void Searcher::reset() {
            pvmove = move::Move();
            stop.store(false, std::memory_order_relaxed);
            tt.clear();
            data.reset();
        }

        void Searcher::resetForSearch() { tt.resetForSearch(); }

        void Searcher::setTranspositionTableSize(const std::size_t size) { tt.setSize(size); }

        SearchResult Searcher::startSearch(
          board::Board&                                    board,
          const SearchInfo&                                info,
          std::function<void(const search::SearchResult&)> searchProgressReportHandler,
          std::function<void(const search::SearchResult&)> searchCompleteReportHander) {
            pvmove = move::Move();
            stop.store(false, std::memory_order_relaxed);
            return searchIteratively(board, info, searchProgressReportHandler,
                                     searchCompleteReportHander);
        }

        SearchResult Searcher::startSearch(board::Board& board, const SearchInfo& info) {
            return startSearch(board, info, [](auto&) {}, [](auto&) {});
        }

        void Searcher::stopSearch() { stop.store(true, std::memory_order_relaxed); }

        void Searcher::shouldStopSearchNow(const SearchInfo& info) {
            if (info.timeset && (utils::currtimeInMilliseconds() >= info.stoptime))
            {
                stop.store(true, std::memory_order_relaxed);
            }
        }

        SearchResult Searcher::searchIteratively(
          board::Board&                                    board,
          const SearchInfo&                                info,
          std::function<void(const search::SearchResult&)> searchProgressReportHandler,
          std::function<void(const search::SearchResult&)> searchCompleteReportHander) {
            SearchResult bestresult{};

            i32 alpha = -INF;
            i32 beta  = INF;

            for (u8 currdepth = 1; currdepth <= info.depth; currdepth++)
            {
                SearchResult result{};

                const u64 starttime = utils::currtimeInMilliseconds();
                i32       score =
                  search<NodeType::PV>(board, currdepth, alpha, beta, info, &result, true);
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

                u8          pv_count = 0;
                tt::TTEntry ttentry;
                bool        tthit = tt.probe(&ttentry, board);
                while (pv_count++ < currdepth && tthit)
                {
                    if (ttentry.flag != tt::TTFlag::EXACT)
                    {
                        break;
                    }
                    const move::Move move = ttentry.move;
                    if (board.doMove(move) == board::DoMoveResult::LEGAL)
                    {
                        result.pv.push_back(move);
                        tthit = tt.probe(&ttentry, board);
                    }
                    else
                    {
                        break;
                    }
                }
                while (board.getPlyCount() > 0)
                {
                    board.undoMove();
                }
#ifdef DEBUG
                assert(board.getPlyCount() == 0);
#endif

                searchProgressReportHandler(result);
            }

            searchCompleteReportHander(bestresult);

            return bestresult;
        }

        template<NodeType nodeType>
        i32 Searcher::search(board::Board&     board,
                             i8                depth,
                             i32               alpha,
                             i32               beta,
                             const SearchInfo& info,
                             SearchResult*     result,
                             const bool        do_null) {
            if ((result->nodes & 2047) == 0)
            {
                shouldStopSearchNow(info);
            }

            const i32 alpha_orig = alpha;

            if (board.getPlyCount() >= MAX_DEPTH - 1) [[unlikely]]
            {
                return eval::evaluateBoard(board);
            }

            if (do_null && board.hasPositionRepeated())
            {
                return 0;
            }

            if (board.getHalfmoveClock() >= 100)
            {
                return 0;
            }

            const bool is_in_check = movegen::isInCheck(board);

            if (is_in_check)
            {
                depth++;
            }

            if (depth <= 0)
            {
                return quiescencesearch(board, alpha, beta, info, result);
            }

            constexpr bool is_pv_node_type = (nodeType != NodeType::NON_PV);
            const bool     is_pv_node      = ((beta - alpha) > 1) || is_pv_node_type;

            if (board.getPlyCount() > 0 && !is_pv_node)
            {
                tt::TTEntry ttentry;
                const bool  tthit = tt.probe(&ttentry, board);
                if (tthit)
                {
                    if (ttentry.depth >= depth)
                    {
                        i32 ttvalue = ttentry.value;

                        if (ttvalue < -MATE_SCORE)
                        {
                            ttvalue += board.getPlyCount();
                        }
                        else if (ttvalue > MATE_SCORE)
                        {
                            ttvalue -= board.getPlyCount();
                        }

                        if (ttentry.flag == tt::TTFlag::EXACT
                            || (ttentry.flag == tt::TTFlag::LOWERBOUND && ttvalue >= beta)
                            || (ttentry.flag == tt::TTFlag::UPPERBOUND && ttvalue <= alpha))
                        {
                            return ttvalue;
                        }
                    }
                }
            }

            // Node Pruning
            if (!is_in_check && !is_pv_node)
            {
                // Reverse Futility Pruning
                if (depth <= searchParams.RFP_DEPTH_MAX)
                {
                    const i32 eval   = eval::evaluateBoard(board);
                    const i32 margin = searchParams.RFP_MARGIN * depth;
                    if (eval >= beta + margin)
                    {
                        return eval;
                    }
                }

                // Null Move Pruning
                if (do_null && depth >= 3 && !eval::isEndGame(board))
                {
                    const u8 r = 2;
#ifdef DEBUG
                    const u64 hash = board.getHash();
#endif
                    board.doNullMove();
                    const i32 score = -search<NodeType::NON_PV>(board, depth - r, -beta, -beta + 1,
                                                                info, result, false);
                    board.undoNullMove();
#ifdef DEBUG
                    assert(hash == board.getHash());
#endif
                    if (score >= beta)
                    {
                        return beta;
                    }
                }
            }

            i32        best_score = -INF;
            move::Move best_move_so_far;
            u32        legal_moves_count = 0;
            u32        moves_searched    = 0;

            containers::ArrayList<move::Move> moves;
            movegen::generatePseudolegalMoves(&moves, board, movegen::MovegenType::ALL);
            scoreMoves(&moves, board, pvmove, tt, data);

            for (u8 i = 0; i < moves.size(); i++)
            {
                sortMoves(&moves, i);
                const move::Move move = moves.at(i);

                const board::DoMoveResult do_move_result = board.doMove(move);

                if (do_move_result == board::DoMoveResult::ILLEGAL)
                {
                    board.undoMove();
                    continue;
                }

                result->nodes++;
                legal_moves_count++;

                i32 score = -INF;

                // PVS + LMR
                if (moves_searched == 0)
                {
                    score =
                      -search<nodeType>(board, depth - 1, -beta, -alpha, info, result, do_null);
                }
                else
                {
                    // clang-format off
                    if (!is_in_check
                        && !is_pv_node
                        && moves_searched >= 4
                        && depth >= 3
                        && !movegen::isInCheck(board))
                    // clang-format on
                    {
                        u8 r = 0;
                        if (move::isCapture(move.getFlag()) || move::isPromotion(move.getFlag()))
                        {
                            float LMR_R_BIAS_T, LMR_R_SCALE_T;
                            LMR_R_BIAS_T  = 0.0f;
                            LMR_R_SCALE_T = 2.75f;
                            r =
                              std::min(static_cast<int>(LMR_R_BIAS_T
                                                        + std::log(depth) * std::log(moves_searched)
                                                            / LMR_R_SCALE_T),
                                       depth - 1);
                        }
                        else
                        {
                            float LMR_R_BIAS_Q, LMR_R_SCALE_Q;
                            LMR_R_BIAS_Q  = 1.0f;
                            LMR_R_SCALE_Q = 1.5f;
                            r =
                              std::min(static_cast<int>(LMR_R_BIAS_Q
                                                        + std::log(depth) * std::log(moves_searched)
                                                            / LMR_R_SCALE_Q),
                                       depth - 1);
                        }
                        score = -search<NodeType::NON_PV>(board, depth - r, -alpha - 1, -alpha,
                                                          info, result, do_null);
                    }
                    else
                    {
                        score = alpha + 1;
                    }

                    if (score > alpha)
                    {
                        score = -search<NodeType::NON_PV>(board, depth - 1, -alpha - 1, -alpha,
                                                          info, result, do_null);
                        if (score > alpha && score < beta)
                        {
                            // re-search
                            score = -search<NodeType::PV>(board, depth - 1, -beta, -alpha, info,
                                                          result, do_null);
                        }
                    }
                }

                moves_searched++;

                board.undoMove();

                if (stop.load(std::memory_order_relaxed))
                {
                    return 0;
                }

                // Fail-soft
                if (score > best_score)
                {
                    best_score = score;
                    if (score > alpha)
                    {
                        alpha            = score;
                        best_move_so_far = move;
                        if (board.getPlyCount() == 0 && !stop.load(std::memory_order_relaxed))
                        {
                            pvmove = move;
                        }
                        if (score >= beta)
                        {
                            if (!move::isCapture(move.getFlag()))
                            {
                                const Piece piece = board.getPiece(move.getFrom());
                                data.history[piece][move.getTo()] += depth;
                            }
                            break;
                        }
                    }
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

            if (!stop.load(std::memory_order_relaxed))
            {
                tt::TTFlag flag = tt::TTFlag::NONE;
                if (best_score <= alpha_orig)
                {
                    flag = tt::TTFlag::UPPERBOUND;
                }
                else if (best_score >= beta)
                {
                    flag = tt::TTFlag::LOWERBOUND;
                }
                else
                {
                    flag = tt::TTFlag::EXACT;
                }
                tt.store(board, depth, flag, best_score, best_move_so_far);
            }

            if (board.getPlyCount() == 0 && !stop.load(std::memory_order_relaxed))
            {
                result->bestmove = pvmove;
            }

            return best_score;
        }

        i32 Searcher::quiescencesearch(
          board::Board& board, i32 alpha, i32 beta, const SearchInfo& info, SearchResult* result) {
            if ((result->nodes & 2047) == 0)
            {
                shouldStopSearchNow(info);
            }

            if (board.getPlyCount() >= MAX_DEPTH - 1)
            {
                return eval::evaluateBoard(board);
            }

            const i32 stand_pat = eval::evaluateBoard(board);
            if (stand_pat >= beta)
            {
                return beta;
            }
            if (alpha < stand_pat)
            {
                alpha = stand_pat;
            }

            containers::ArrayList<move::Move> moves;
            movegen::generatePseudolegalMoves(&moves, board, movegen::MovegenType::CAPTURES);
            scoreMoves(&moves, board, pvmove, tt, data);

            for (u8 i = 0; i < moves.size(); i++)
            {
                sortMoves(&moves, i);
                const move::Move move = moves.at(i);

                const board::DoMoveResult do_move_result = board.doMove(move);

                if (do_move_result == board::DoMoveResult::ILLEGAL)
                {
                    board.undoMove();
                    continue;
                }

                result->nodes++;

                const i32 score = -quiescencesearch(board, -beta, -alpha, info, result);

                board.undoMove();

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
