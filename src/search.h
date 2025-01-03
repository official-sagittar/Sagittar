#pragma once

#include "board.h"
#include "move.h"
#include "pch.h"
#include "types.h"

namespace sagittar {

    namespace search {

        constexpr u32 INF        = 50000;
        constexpr u32 MATE_VALUE = 49000;
        constexpr u32 MATE_SCORE = 48000;
        constexpr u8  MAX_DEPTH  = 64;

        struct SearchInfo {
            i8   depth;
            u64  starttime;
            u64  stoptime;
            bool timeset;
        };

        struct SearchResult {
            i32                     score;
            bool                    is_mate;
            u8                      mate_in;
            u8                      depth;
            u64                     nodes;
            u64                     time;
            std::vector<move::Move> pv;
            move::Move              bestmove{};
        };

        class Searcher {
           private:
            std::atomic_bool stop;

           private:
            void shouldStopSearchNow(const SearchInfo&);
            i32  search(board::Board&, i8 depth, const SearchInfo&, SearchResult*);
            SearchResult
            searchRoot(board::Board&                                    board,
                       const SearchInfo&                                info,
                       std::function<void(const search::SearchResult&)> searchProgressReportHandler,
                       std::function<void(const search::SearchResult&)> searchCompleteReportHander);

           public:
            Searcher();
            void         reset();
            SearchResult startSearch(
              board::Board&                                    board,
              const SearchInfo&                                info,
              std::function<void(const search::SearchResult&)> searchProgressReportHandler,
              std::function<void(const search::SearchResult&)> searchCompleteReportHander);
            SearchResult startSearch(board::Board& board, const SearchInfo& info);
            void         stopSearch();
        };

    }

}
