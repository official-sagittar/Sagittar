#pragma once

#include "core/move.h"
#include "core/position.h"
#include "pch.h"
#include "search/search.h"
#include "search/types.h"

namespace sagittar {

    class Engine {
       public:
        Engine();
        void reset();
        void set_tt_size_mb(const size_t tt_size_mb);
        bool set_fen(std::string fen);
        bool do_move(const core::Move move);
        bool do_move(const std::string& move_str);
        void perft(const int depth);
        void reset_for_search();
        search::SearchResult
             search(search::SearchInfo                               info,
                    std::function<void(const search::SearchResult&)> progress_handler,
                    std::function<void(const search::SearchResult&)> complete_hander);
        void stop_search();
        void display_position() const;
        ~Engine() = default;

       private:
        core::Position   pos;
        search::Searcher searcher;
        size_t           tt_size_mb;
    };

}
