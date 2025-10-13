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

        Searcher::ThreadData::ThreadData() {
            key_history.reserve(1024);
            key_history.shrink_to_fit();
            key_history.clear();
        }

        board::DoMoveResult Searcher::ThreadData::doMove(board::Board&    board,
                                                         const move::Move move) {
            key_history.push_back(board.getHash());
            return board.doMove(move);
        }

        void Searcher::ThreadData::doNullMove(board::Board& board) {
            key_history.push_back(board.getHash());
            board.doNullMove();
        }

        void Searcher::ThreadData::undoMove() { key_history.pop_back(); }

        void Searcher::ThreadData::undoNullMove() { key_history.pop_back(); }

        /*
        * Searcher
        */

        Searcher::Searcher() { reset(); }

        void Searcher::reset() {
            pvmove = move::Move();
            stop.store(false, std::memory_order_relaxed);
            tt.clear();
            data.reset();
        }

        void Searcher::resetForSearch() { tt.resetForSearch(); }

        void Searcher::setTranspositionTableSize(const std::size_t size) { tt.setSize(size); }

        SearchResult
        Searcher::startSearch(const board::Board&                      board,
                              std::span<u64>                           key_history,
                              SearchInfo                               info,
                              std::function<void(const SearchResult&)> searchProgressReportHandler,
                              std::function<void(const SearchResult&)> searchCompleteReportHander) {
            pvmove = move::Move();
            stop.store(false, std::memory_order_relaxed);
            timeman::setSearchHardBoundTime(&info, board);

            ThreadData thread{};
            std::ranges::copy(key_history, std::back_inserter(thread.key_history));

            return searchIteratively(board, thread, info, searchProgressReportHandler,
                                     searchCompleteReportHander);
        }

        SearchResult Searcher::startSearch(const board::Board& board,
                                           std::span<u64>      key_history,
                                           SearchInfo          info) {
            return startSearch(board, key_history, info, [](auto&) {}, [](auto&) {});
        }

        void Searcher::stopSearch() { stop.store(true, std::memory_order_relaxed); }

        void Searcher::shouldStopSearchNow(const SearchInfo& info) {
            if (info.timeset && (utils::currtimeInMilliseconds() >= info.stoptime))
            {
                stop.store(true, std::memory_order_relaxed);
            }
        }

        SearchResult Searcher::searchIteratively(
          const board::Board&                      board,
          ThreadData&                              thread,
          const SearchInfo&                        info,
          std::function<void(const SearchResult&)> searchProgressReportHandler,
          std::function<void(const SearchResult&)> searchCompleteReportHander) {
            SearchResult bestresult{};

            Score alpha = -INF;
            Score beta  = INF;

            for (Depth currdepth = 1; currdepth <= info.depth; currdepth++)
            {
                SearchResult result{};

                const u64 starttime = utils::currtimeInMilliseconds();
                Score score = search<NodeType::PV>(board, currdepth, alpha, beta, 0, thread, info,
                                                   &result, true);
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
                result.depth    = currdepth;
                result.time     = time;
                result.hashfull = tt.hashfull();
                result.pv       = {result.bestmove};

                searchProgressReportHandler(result);
            }

            searchCompleteReportHander(bestresult);

            return bestresult;
        }

        template<NodeType nodeType>
        Score Searcher::search(const board::Board& board,
                               Depth               depth,
                               Score               alpha,
                               Score               beta,
                               const i32           ply,
                               ThreadData&         thread,
                               const SearchInfo&   info,
                               SearchResult*       result,
                               const bool          do_null) {
            if ((result->nodes & 2047) == 0)
            {
                shouldStopSearchNow(info);
                if (ply > 0 && stop.load(std::memory_order_relaxed))
                {
                    return 0;
                }
            }

            if (ply >= MAX_DEPTH - 1) [[unlikely]]
            {
                return eval::evaluateBoard(board);
            }

            if (ply > 0
                && ((do_null && board.hasPositionRepeated(thread.key_history))
                    || (board.getHalfmoveClock() >= 100)))
            {
                return 0;
            }

            const bool is_in_check = board.isInCheck();

            if (is_in_check)
            {
                depth++;
            }

            if (depth <= 0)
            {
                return quiescencesearch(board, alpha, beta, ply, thread, info, result);
            }

            constexpr bool is_pv_node_type = (nodeType != NodeType::NON_PV);
            const bool     is_pv_node      = ((beta - alpha) > 1) || is_pv_node_type;

            const bool is_critical_node = is_pv_node || is_in_check;

            tt::TTData ttdata;
            const bool tthit = tt.probe(&ttdata, board.getHash());

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
                const Score static_eval = eval::evaluateBoard(board);

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
                if (do_null && depth >= 3 && !eval::isEndGame(board))
                {
                    const u8 r = 2;
#ifdef DEBUG
                    const u64 hash = board.getHash();
#endif
                    board::Board board_copy = board;
                    thread.doNullMove(board_copy);
                    const Score score = -search<NodeType::NON_PV>(
                      board_copy, depth - r, -beta, -beta + 1, ply, thread, info, result, false);
                    thread.undoNullMove();
#ifdef DEBUG
                    assert(hash == board.getHash());
#endif
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

            // Generate pseudolegal mooves
            containers::ArrayList<move::Move> moves;
            movegen::generatePseudolegalMoves<movegen::MovegenType::ALL>(&moves, board);

            // Score moves
            const move::Move ttmove = tthit ? ttdata.move : move::Move();
            scoreMoves(&moves, board, pvmove, ttmove, data, ply);

            for (u8 i = 0; i < moves.size(); i++)
            {
                sortMoves(&moves, i);
                const move::Move move            = moves.at(i);
                const Piece      move_piece      = board.getPiece(move.getFrom());
                const PieceType  move_piece_type = pieceTypeOf(move_piece);

                board::Board              board_copy     = board;
                const board::DoMoveResult do_move_result = thread.doMove(board_copy, move);
                if (do_move_result == board::DoMoveResult::ILLEGAL)
                {
                    thread.undoMove();
                    continue;
                }

                legal_moves_count++;

                const bool move_is_quite =
                  !(move::isCapture(move.getFlag()) || move::isPromotion(move.getFlag()));
                const bool move_gives_check = board_copy.isInCheck();

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
                          moves.size() * (1 - (params::lmp_treshold_pct - (0.1 * depth)));
                        if (moves_searched >= LMP_MOVE_CUTOFF)
                        {
                            thread.undoMove();
                            continue;
                        }
                    }
                }

                result->nodes++;

                Score score = -INF;

                // PVS + LMR
                if (moves_searched == 0)
                {
                    score = -search<nodeType>(board_copy, depth - 1, -beta, -alpha, ply + 1, thread,
                                              info, result, do_null);
                }
                else
                {
                    // Late Move Reduction
                    // clang-format off
                    if (depth >= 3
                        && moves_searched >= 4
                        && !is_critical_node
                        && !move_gives_check
                        && move.getScore() != KILLER_0_SCORE
                        && move.getScore() != KILLER_1_SCORE
                    )
                    // clang-format on
                    {
                        u8 r = 0;
                        if (move_is_quite)
                        {
                            r =
                              params::lmr_r_table_quiet[std::min(moves_searched, 64U)][(int) depth];
                        }
                        else
                        {
                            r = params::lmr_r_table_tactical[std::min(moves_searched, 64U)]
                                                            [(int) depth];
                        }
                        score = -search<NodeType::NON_PV>(board_copy, depth - r, -alpha - 1, -alpha,
                                                          ply + 1, thread, info, result, do_null);
                    }
                    else
                    {
                        score = alpha + 1;
                    }

                    if (score > alpha)
                    {
                        score = -search<NodeType::NON_PV>(board_copy, depth - 1, -alpha - 1, -alpha,
                                                          ply + 1, thread, info, result, do_null);
                        if (score > alpha && score < beta)
                        {
                            // re-search
                            score = -search<NodeType::PV>(board_copy, depth - 1, -beta, -alpha,
                                                          ply + 1, thread, info, result, do_null);
                        }
                    }
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
                        if (ply == 0 && !stop.load(std::memory_order_relaxed))
                        {
                            pvmove = move;
                        }
                        if (best_score >= beta)
                        {
                            if (!move::isCapture(move.getFlag()))
                            {
                                // Killer Heuristic
                                data.killer_moves[1][ply] = data.killer_moves[0][ply];
                                data.killer_moves[0][ply] = move;

                                // History Heuristic
                                data.history[move_piece][move.getTo()] += depth;
                            }
                            ttflag = tt::TTFlag::LOWERBOUND;
                            break;
                        }
                    }
                }
            }

            if (legal_moves_count == 0)
            {
                if (is_in_check)
                {
                    return -MATE_VALUE + ply;
                }
                else
                {
                    return 0;
                }
            }

            if (!stop.load(std::memory_order_relaxed))
            {
                tt.store(board.getHash(), ply, depth, ttflag, best_score, best_move_so_far);

                if (ply == 0)
                {
                    result->bestmove = pvmove;
                }
            }

            return best_score;
        }

        Score Searcher::quiescencesearch(const board::Board& board,
                                         Score               alpha,
                                         Score               beta,
                                         const i32           ply,
                                         ThreadData&         thread,
                                         const SearchInfo&   info,
                                         SearchResult*       result) {
            if ((result->nodes & 2047) == 0)
            {
                shouldStopSearchNow(info);
                if (ply > 0 && stop.load(std::memory_order_relaxed))
                {
                    return 0;
                }
            }

            if (ply >= MAX_DEPTH - 1)
            {
                return eval::evaluateBoard(board);
            }

            const Score stand_pat = eval::evaluateBoard(board);
            if (stand_pat >= beta)
            {
                return beta;
            }
            if (alpha < stand_pat)
            {
                alpha = stand_pat;
            }

            containers::ArrayList<move::Move> moves;
            movegen::generatePseudolegalMoves<movegen::MovegenType::CAPTURES>(&moves, board);

            tt::TTData       ttdata;
            const bool       tthit  = tt.probe(&ttdata, board.getHash());
            const move::Move ttmove = tthit ? ttdata.move : move::Move();

            scoreMoves(&moves, board, pvmove, ttmove, data, ply);

            for (u8 i = 0; i < moves.size(); i++)
            {
                sortMoves(&moves, i);
                const move::Move move = moves.at(i);

                board::Board              board_copy     = board;
                const board::DoMoveResult do_move_result = thread.doMove(board_copy, move);

                if (do_move_result == board::DoMoveResult::ILLEGAL)
                {
                    thread.undoMove();
                    continue;
                }

                result->nodes++;

                const Score score =
                  -quiescencesearch(board_copy, -beta, -alpha, ply + 1, thread, info, result);

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
