#pragma once

#include "commons/pch.h"
#include "core/move.h"
#include "core/position.h"
#include "core/types.h"
#include "search/tt.h"
#include "search/types.h"

namespace sagittar::search {

    constexpr Score INF        = 15000;
    constexpr Score MATE_VALUE = 14000;
    constexpr Score MATE_SCORE = 13000;
    constexpr Score WIN_SCORE  = 12000;
    constexpr Depth MAX_DEPTH  = 64;

    constexpr std::size_t DEFAULT_TT_SIZE_MB = 16;

    enum class NodeType {
        NON_PV,
        ROOT,
        PV
    };

    struct SearcherData {
        u32  history[15][64];  // [piece][to]
        Move killer_moves[2][MAX_DEPTH];

        SearcherData();
        void reset();
    };

    class Searcher {
       private:
        std::atomic_bool   stop;
        TranspositionTable tt = TranspositionTable(DEFAULT_TT_SIZE_MB);
        SearcherData       data;

        struct ThreadData {
            std::vector<u64> key_history;
            Move             pvmove{};
            size_t           nodes;

            ThreadData();
            bool doMove(Position& pos, const Move& move);
            void doNullMove(Position& pos);
            void undoMove();
            void undoNullMove();
        };

       private:
        void shouldStopSearchNow(const SearchInfo&);

        SearchResult
        searchIteratively(const Position&                          pos,
                          ThreadData&                              thread,
                          const SearchInfo&                        info,
                          std::function<void(const SearchResult&)> searchProgressReportHandler,
                          std::function<void(const SearchResult&)> searchCompleteReportHander);

        template<NodeType nodeType>
        Score search(const Position&   pos,
                     Depth             depth,
                     Score             alpha,
                     Score             beta,
                     const i32         ply,
                     ThreadData&       thread,
                     const SearchInfo& info,
                     const bool        do_null);

        Score quiescencesearch(const Position&   pos,
                               Score             alpha,
                               Score             beta,
                               const i32         ply,
                               ThreadData&       thread,
                               const SearchInfo& info);

       public:
        Searcher();

        void reset();

        void resetForSearch();

        void setTranspositionTableSize(const std::size_t);

        SearchResult
        startSearch(const Position&                          pos,
                    std::span<u64>                           key_history,
                    SearchInfo                               info,
                    std::function<void(const SearchResult&)> searchProgressReportHandler,
                    std::function<void(const SearchResult&)> searchCompleteReportHander);

        SearchResult startSearch(const Position& pos, std::span<u64> key_history, SearchInfo info);

        void stopSearch();
    };

}
