#pragma once

#include "board.h"
#include "pch.h"
#include "search.h"

namespace sagittar {

    class Engine {
       private:
        std::string      name;
        std::string      version;
        board::Board     board;
        search::Searcher searcher;

       public:
        Engine();
        Engine(const Engine& engine);
        ~Engine();
        std::string         getName() const;
        std::string         getVersion() const;
        void                reset();
        void                resetForSearch();
        void                setStartpos();
        void                setPositionFromFEN(std::string);
        board::DoMoveResult doMove(const std::string&);
        search::SearchResult
                             search(const search::SearchInfo&,
                                    std::function<void(const search::SearchResult&)> searchProgressReportHandler,
                                    std::function<void(const search::SearchResult&)> searchCompleteReportHander);
        search::SearchResult search(const search::SearchInfo&);
        void                 stopSearch();
        void                 bench();
        void                 displayBoard() const;
    };

}
