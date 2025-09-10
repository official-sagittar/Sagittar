#pragma once

#include "core/defs.h"
#include "core/position.h"
#include "core/tt.h"
#include "pch.h"
#include "search/types.h"

namespace sagittar {

    namespace search {

        using namespace sagittar::core;

        class Searcher {
           public:
            Searcher();
            void         reset();
            void         reset_for_search();
            void         set_tt_size(const size_t mb);
            void         stop();
            SearchResult start(Position const*                          pos,
                               PositionHistory* const                   history,
                               SearchInfo                               info,
                               std::function<void(const SearchResult&)> progress_handler,
                               std::function<void(const SearchResult&)> complete_hander);
            SearchResult
            start(Position const* pos, PositionHistory* const history, SearchInfo info);

           private:
            void         check_timeup(const SearchInfo& info);
            SearchResult search(Position const*                          pos,
                                PositionHistory* const                   history,
                                const SearchInfo&                        info,
                                std::function<void(const SearchResult&)> progress_handler,
                                std::function<void(const SearchResult&)> complete_hander);
            Score        search_root(Position const*        pos,
                                     int                    depth,
                                     Score                  alpha,
                                     Score                  beta,
                                     PositionHistory* const history,
                                     const SearchInfo&      info,
                                     SearchResult*          result);
            Score        search_alphabeta(Position const*        pos,
                                          int                    depth,
                                          Score                  alpha,
                                          Score                  beta,
                                          const int              ply,
                                          PositionHistory* const history,
                                          const SearchInfo&      info,
                                          SearchResult*          result);
            Score        search_quiescence(Position const*        pos,
                                           Score                  alpha,
                                           Score                  beta,
                                           const int              ply,
                                           PositionHistory* const history,
                                           const SearchInfo&      info,
                                           SearchResult*          result);

            std::atomic_bool                                      stopped;
            TranspositionTable<TTClient::SEARCH, uint64_t, Score> tt;
            Move                                                  pv_move;
        };

    }

}
