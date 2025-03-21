#pragma once

#include "commons/pch.h"
#include "commons/types.h"
#include "sagittar/core/board.h"
#include "sagittar/core/move.h"
#include "sagittar/core/types.h"
#include "sagittar/search/constants.h"
#include "sagittar/search/data.h"
#include "sagittar/search/tt.h"
#include "sagittar/search/types.h"

namespace sagittar {

    namespace search {

        using namespace commons::types;
        using namespace core::types;
        using namespace search::types;

        enum class NodeType {
            NON_PV,
            PV
        };

        class Searcher {
           private:
            core::move::Move       pvmove;
            std::atomic_bool       stop;
            tt::TranspositionTable tt = tt::TranspositionTable(constants::DEFAULT_TT_SIZE_MB);
            data::SearcherData     data;

           private:
            void shouldStopSearchNow(const SearchInfo&);

            SearchResult
            searchIteratively(core::board::Board&                      board,
                              const SearchInfo&                        info,
                              std::function<void(const SearchResult&)> searchProgressReportHandler,
                              std::function<void(const SearchResult&)> searchCompleteReportHander);

            template<NodeType nodeType>
            Score search(core::board::Board& board,
                         Depth               depth,
                         Score               alpha,
                         Score               beta,
                         const i32           ply,
                         const SearchInfo&   info,
                         SearchResult*       result,
                         const bool          do_null);

            Score quiescencesearch(core::board::Board& board,
                                   Score               alpha,
                                   Score               beta,
                                   const i32           ply,
                                   const SearchInfo&   info,
                                   SearchResult*       result);

           public:
            Searcher();

            void reset();

            void resetForSearch();

            void setTranspositionTableSize(const std::size_t);

            SearchResult
            startSearch(core::board::Board&                      board,
                        SearchInfo                               info,
                        std::function<void(const SearchResult&)> searchProgressReportHandler,
                        std::function<void(const SearchResult&)> searchCompleteReportHander);

            SearchResult startSearch(core::board::Board& board, SearchInfo info);

            void stopSearch();
        };

    }  // namespace search

}  // namespace sagittar
