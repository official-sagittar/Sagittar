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
        std::string      name;
        Position         pos;
        search::Searcher searcher;
        std::vector<u64> key_history;

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
               std::function<void(const search::SearchResult&)> searchProgressReportHandler,
               std::function<void(const search::SearchResult&)> searchCompleteReportHander);

        void stopSearch();

        void bench();

        void tune();

        void display() const;
    };

}
