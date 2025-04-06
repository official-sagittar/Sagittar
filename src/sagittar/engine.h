#pragma once

#include "commons/pch.h"
#include "sagittar/core/board.h"
#include "sagittar/core/move.h"
#include "sagittar/search/search.h"
#include "sagittar/search/types.h"

namespace sagittar {

    class Engine {
       private:
        std::string        name;
        std::string        version;
        core::board::Board board;
        search::Searcher   searcher;

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

        core::board::DoMoveResult doMove(const std::string&);

        core::board::DoMoveResult doMove(const core::move::Move&);

        search::types::SearchResult search(search::types::SearchInfo);

        search::types::SearchResult
        search(search::types::SearchInfo,
               std::function<void(const search::types::SearchResult&)> searchProgressReportHandler,
               std::function<void(const search::types::SearchResult&)> searchCompleteReportHander);

        void stopSearch();

        void bench();

        void displayBoard() const;
    };

}
