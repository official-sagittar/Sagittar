#pragma once

#include "core/defs.h"
#include "core/position.h"
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
            void         stop();
            SearchResult start(Position const*                          pos,
                               SearchInfo                               info,
                               std::function<void(const SearchResult&)> progress_handler,
                               std::function<void(const SearchResult&)> complete_hander);
            SearchResult start(Position const* pos, SearchInfo info);

           private:
            void         check_timeup(const SearchInfo& info);
            SearchResult search(Position const*                          pos,
                                const SearchInfo&                        info,
                                std::function<void(const SearchResult&)> progress_handler,
                                std::function<void(const SearchResult&)> complete_hander);
            Score        search_root(Position const*   pos,
                                     int               depth,
                                     const int         ply,
                                     const SearchInfo& info,
                                     SearchResult*     result);
            Score        search_negamax(Position const*   pos,
                                        int               depth,
                                        const int         ply,
                                        const SearchInfo& info,
                                        SearchResult*     result);

            std::atomic_bool stopped;
        };

    }

}
