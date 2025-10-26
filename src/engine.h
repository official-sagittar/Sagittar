#pragma once

#include "move.h"
#include "pch.h"
#include "position.h"
#include "search.h"
#include "searchtypes.h"
#include "types.h"

namespace sagittar {

    class Engine {
       private:
        std::string      name;
        std::string      version;
        core::Position   pos;
        search::Searcher searcher;
        std::vector<u64> key_history;

       public:
        Engine();

        ~Engine();

        std::string getName() const;

        void reset();

        void resetForSearch();

        void setTranspositionTableSize(const std::size_t);

        void setStartpos();

        void setPositionFromFEN(std::string);

        std::string getPositionAsFEN();

        core::DoMoveResult doMove(const std::string&);

        core::DoMoveResult doMove(const move::Move&);

        search::SearchResult search(search::SearchInfo);

        search::SearchResult
        search(search::SearchInfo,
               std::function<void(const search::SearchResult&)> searchProgressReportHandler,
               std::function<void(const search::SearchResult&)> searchCompleteReportHander);

        void stopSearch();

        void bench();

        void display() const;
    };

}
