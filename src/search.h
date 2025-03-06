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
            u32        history[15][64];  // [piece][to]
            move::Move killer_moves[2][MAX_DEPTH];

            SearcherData();
            void reset();
        };

        struct SearcherParams {
            i8  rfp_depth_max;
            i32 rfp_margin;

            i8 nmp_depth_min;

            i8     lmp_depth_max;
            double lmp_treshold;

            i8  lmr_depth_min;
            u32 lmr_movesearched_min;
            u8  lmr_r_table_tactical[64][64];  // [move][depth]
            u8  lmr_r_table_quiet[64][64];     // [move][depth]
        };

        class Searcher {
           private:
            move::Move             pvmove;
            std::atomic_bool       stop;
            tt::TranspositionTable tt = tt::TranspositionTable(DEFAULT_TT_SIZE_MB);
            SearcherData           data;
            SearcherParams         search_params;

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

            void reset();

            void resetForSearch();

            void setTranspositionTableSize(const std::size_t);

            void setParams();

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
