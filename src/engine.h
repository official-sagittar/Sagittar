#pragma once

#include "commons/pch.h"
#include "core/move.h"
#include "core/position.h"
#include "core/types.h"
#include "search/search.h"
#include "search/types.h"

namespace sagittar {

    class Engine {
       private:
        std::string      m_name;
        Position         m_pos;
        search::Searcher m_searcher;
        std::vector<U64> m_key_history;

       public:
        Engine();

        ~Engine();

        std::string getName() const;

        void reset();

        void resetForSearch();

        void setTranspositionTableSize(const std::size_t);

        void setThreadCount(const std::size_t);

        void setPosition(std::string);

        bool doMove(const std::string&);

        void perft(const Depth) const;

        search::SearchResult search(search::SearchInfo);

        search::SearchResult
        search(search::SearchInfo,
               std::function<void(const search::SearchResult&)> search_progress_report_handler,
               std::function<void(const search::SearchResult&)> search_complete_report_hander);

        void stopSearch();

        void bench();

#ifdef EXTERNAL_TUNE
        void tune(const std::filesystem::path& epd_path);
#endif

        void display() const;
    };

}
