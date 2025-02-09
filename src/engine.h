#pragma once

#include "board.h"
#include "move.h"
#include "params.h"
#include "pch.h"
#include "search.h"

namespace sagittar {

    class Engine {
       private:
        std::string        name;
        std::string        version;
        board::Board       board;
        search::Searcher   searcher;
        params::Parameters params;

       public:
        Engine();
        ~Engine();
        std::string         getName() const;
        void                reset();
        void                setParamsInt(const std::string key, const int value);
        void                setParamsFloat(const std::string key, const float value);
        void                resetForSearch();
        void                setStartpos();
        void                setPositionFromFEN(std::string);
        std::string         getPositionAsFEN();
        board::DoMoveResult doMove(const std::string&);
        board::DoMoveResult doMove(const move::Move&);
        void                setTranspositionTableSize(const std::size_t);
        void                setSearcherParams();
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
