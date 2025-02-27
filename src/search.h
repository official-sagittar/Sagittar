#pragma once

#include "board.h"
#include "move.h"
#include "params.h"
#include "pch.h"
#include "searchtypes.h"
#include "tt.h"
#include "types.h"

namespace sagittar {

    namespace search {

        constexpr u32 INF        = 50000;
        constexpr i32 MATE_VALUE = 49000;
        constexpr i32 MATE_SCORE = 48000;
        constexpr u8  MAX_DEPTH  = 64;

        constexpr std::size_t DEFAULT_TT_SIZE_MB = 16;

        enum class NodeType {
            NON_PV,
            PV
        };

        struct SearcherData {
            u32 history[15][64];  // [piece][to]

            SearcherData();
            void reset();
        };

        struct SearcherParams {
            int RFP_DEPTH_MAX;
            int RFP_MARGIN;
        };

        class Searcher {
           private:
            move::Move             pvmove;
            std::atomic_bool       stop;
            tt::TranspositionTable tt = tt::TranspositionTable(DEFAULT_TT_SIZE_MB);
            SearcherData           data;
            SearcherParams         searchParams;

           private:
            void shouldStopSearchNow(const SearchInfo&);

            SearchResult
            searchIteratively(board::Board&                            board,
                              const SearchInfo&                        info,
                              std::function<void(const SearchResult&)> searchProgressReportHandler,
                              std::function<void(const SearchResult&)> searchCompleteReportHander);

            template<NodeType nodeType>
            i32 search(board::Board&     board,
                       i8                depth,
                       i32               alpha,
                       i32               beta,
                       const i32         ply,
                       const SearchInfo& info,
                       SearchResult*     result,
                       const bool        do_null);

            i32 quiescencesearch(board::Board&     board,
                                 i32               alpha,
                                 i32               beta,
                                 const i32         ply,
                                 const SearchInfo& info,
                                 SearchResult*     result);

           public:
            Searcher();

            void setParams(const parameters::ParameterStore&);

            void reset();

            void resetForSearch();

            void setTranspositionTableSize(const std::size_t);

            SearchResult
            startSearch(board::Board&                            board,
                        SearchInfo                               info,
                        std::function<void(const SearchResult&)> searchProgressReportHandler,
                        std::function<void(const SearchResult&)> searchCompleteReportHander);

            SearchResult startSearch(board::Board& board, SearchInfo info);

            void stopSearch();
        };

    }

}
