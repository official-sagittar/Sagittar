#pragma once

#include "commons/pch.h"
#include "core/move.h"
#include "core/position.h"
#include "core/types.h"
#include "search/searchtypes.h"
#include "search/tt.h"

namespace sagittar {

    namespace search {

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
                core::DoMoveResult doMove(core::Position& pos, const move::Move& move);
                void               doNullMove(core::Position& pos);
                void               undoMove();
                void               undoNullMove();
            };

           private:
            void shouldStopSearchNow(const SearchInfo&);

            SearchResult
            searchIteratively(const core::Position&                    pos,
                              ThreadData&                              thread,
                              const SearchInfo&                        info,
                              std::function<void(const SearchResult&)> searchProgressReportHandler,
                              std::function<void(const SearchResult&)> searchCompleteReportHander);

            template<NodeType nodeType>
            Score search(const core::Position& pos,
                         Depth                 depth,
                         Score                 alpha,
                         Score                 beta,
                         const i32             ply,
                         ThreadData&           thread,
                         const SearchInfo&     info,
                         const bool            do_null);

            Score quiescencesearch(const core::Position& pos,
                                   Score                 alpha,
                                   Score                 beta,
                                   const i32             ply,
                                   ThreadData&           thread,
                                   const SearchInfo&     info);

           public:
            Searcher();

            void reset();

            void resetForSearch();

            void setTranspositionTableSize(const std::size_t);

            SearchResult
            startSearch(const core::Position&                    pos,
                        std::span<u64>                           key_history,
                        SearchInfo                               info,
                        std::function<void(const SearchResult&)> searchProgressReportHandler,
                        std::function<void(const SearchResult&)> searchCompleteReportHander);

            SearchResult
            startSearch(const core::Position& pos, std::span<u64> key_history, SearchInfo info);

            void stopSearch();
        };

    }

}
