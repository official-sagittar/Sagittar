#pragma once

#include "core/defs.h"
#include "core/position.h"
#include "pch.h"
#include "search/types.h"

namespace sagittar {

    namespace search {

        class Searcher {
           public:
            Searcher();
            void         reset();
            void         reset_for_search();
            void         stop();
            SearchResult start(core::Position const*                    pos,
                               SearchInfo                               info,
                               std::function<void(const SearchResult&)> progress_handler,
                               std::function<void(const SearchResult&)> complete_hander);
            SearchResult start(core::Position const* pos, SearchInfo info);

           private:
            void         check_timeup(const SearchInfo& info);
            SearchResult search(core::Position const*                    pos,
                                SearchInfo                               info,
                                std::function<void(const SearchResult&)> progress_handler,
                                std::function<void(const SearchResult&)> complete_hander);
            core::Score
            search_random(core::Position const* pos, const SearchInfo info, SearchResult* result);

            std::atomic_bool stopped;
        };

    }

}
