#pragma once

#include "board.h"
#include "move.h"
#include "params.h"
#include "pch.h"
#include "search.h"
#include "searchtypes.h"

namespace sagittar {

    class Engine {
       private:
        std::string                name;
        std::string                version;
        board::Board               board;
        search::Searcher           searcher;
        parameters::ParameterStore params;

       public:
        Engine();

        ~Engine();

        std::string getName() const;

        void reset();

        void resetForSearch();

        void setTranspositionTableSize(const std::size_t);

        template<typename T>
        void setParam(const std::string& key, const T value) {
            params.set<T>(key, value);
        }

        void setSearcherParams();

        void setStartpos();

        void setPositionFromFEN(std::string);

        std::string getPositionAsFEN();

        board::DoMoveResult doMove(const std::string&);

        board::DoMoveResult doMove(const move::Move&);

        search::SearchResult search(search::SearchInfo);

        search::SearchResult
        search(search::SearchInfo,
               std::function<void(const search::SearchResult&)> searchProgressReportHandler,
               std::function<void(const search::SearchResult&)> searchCompleteReportHander);

        void stopSearch();

        void bench();

        void displayBoard() const;
    };

}
