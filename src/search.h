#pragma once

#include "board.h"
#include "move.h"
#include "pch.h"
#include "searchtypes.h"
#include "tt.h"
#include "types.h"

namespace sagittar {

    namespace search {

        constexpr Score INF        = 15000;
        constexpr Score MATE_VALUE = 14000;
        constexpr Score MATE_SCORE = 13000;
        constexpr Score WIN_SCORE  = 12000;
        constexpr Depth MAX_DEPTH  = 64;

        constexpr std::size_t DEFAULT_TT_SIZE_MB = 16;

        enum class NodeType : u8 {
            NON_PV,
            ROOT,
            PV
        };

        struct SearcherData {
            u32        history[15][64];  // [piece][to]
            move::Move killer_moves[2][MAX_DEPTH];

            SearcherData();
            void reset();
        };

        class Searcher {
           private:
            std::atomic_bool       stop;
            tt::TranspositionTable tt = tt::TranspositionTable(DEFAULT_TT_SIZE_MB);
            SearcherData           data;

            struct ThreadData {
                std::vector<u64> key_history;
                move::Move       pvmove{};
                size_t           nodes;

                ThreadData();
                board::DoMoveResult doMove(board::Board& board, const move::Move& move);
                void                doNullMove(board::Board& board);
                void                undoMove();
                void                undoNullMove();
            };

           private:
            void shouldStopSearchNow(const SearchInfo&);

            SearchResult searchIteratively(
              const board::Board&                        board,
              ThreadData&                                thread,
              const SearchInfo&                          info,
              std::function<void(const SearchResult&)>&& searchProgressReportHandler,
              std::function<void(const SearchResult&)>&& searchCompleteReportHander);

            template<NodeType nodeType>
            Score search(const board::Board& board,
                         Depth               depth,
                         Score               alpha,
                         Score               beta,
                         const i32           ply,
                         ThreadData&         thread,
                         const SearchInfo&   info,
                         const bool          do_null);

            Score quiescencesearch(const board::Board& board,
                                   Score               alpha,
                                   Score               beta,
                                   const i32           ply,
                                   ThreadData&         thread,
                                   const SearchInfo&   info);

           public:
            Searcher();

            void reset();

            void resetForSearch();

            void setTranspositionTableSize(const std::size_t);

            SearchResult
            startSearch(const board::Board&                      board,
                        std::span<u64>                           key_history,
                        SearchInfo                               info,
                        std::function<void(const SearchResult&)> searchProgressReportHandler,
                        std::function<void(const SearchResult&)> searchCompleteReportHander);

            SearchResult
            startSearch(const board::Board& board, std::span<u64> key_history, SearchInfo info);

            void stopSearch();
        };

    }

}
