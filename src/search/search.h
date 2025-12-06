#pragma once

#include "commons/pch.h"
#include "core/move.h"
#include "core/position.h"
#include "core/types.h"
#include "search/history.h"
#include "search/tt.h"
#include "search/types.h"

namespace sagittar::search {

    constexpr Score INF        = 15000;
    constexpr Score MATE_VALUE = 14000;
    constexpr Score MATE_SCORE = 13000;
    constexpr Score WIN_SCORE  = 12000;
    constexpr Depth MAX_DEPTH  = 64;

    constexpr std::size_t DEFAULT_TT_SIZE_MB = 16;

    class Searcher {
       public:
        Searcher();
        Searcher(const Searcher&)            = delete;
        Searcher(Searcher&&)                 = delete;
        Searcher& operator=(const Searcher&) = delete;
        Searcher& operator=(Searcher&&)      = delete;
        ~Searcher()                          = default;

        void reset();
        void resetForSearch();

        void setTranspositionTableSize(const std::size_t);

        [[nodiscard]] SearchResult startSearch(const Position&                          pos,
                                               std::span<u64>                           key_history,
                                               SearchInfo                               info,
                                               std::function<void(const SearchResult&)> onProgress,
                                               std::function<void(const SearchResult&)> onComplete);

        [[nodiscard]] SearchResult
        startSearch(const Position& pos, std::span<u64> key_history, SearchInfo info);

        void stopSearch();

       private:
        class Worker {
           public:
            Worker() = delete;
            Worker(const i32, std::span<u64>, const SearchInfo&, TranspositionTable&);
            Worker(const Worker&)            = delete;
            Worker(Worker&&)                 = delete;
            Worker& operator=(const Worker&) = delete;
            Worker& operator=(Worker&&)      = delete;
            ~Worker()                        = default;

            inline bool isMain() const { return id == 0; }

            [[nodiscard]] SearchResult start(const Position&                          pos,
                                             std::function<void(const SearchResult&)> onProgress,
                                             std::function<void(const SearchResult&)> onComplete);

            void stop();

           private:
            enum class NodeType {
                NON_PV,
                ROOT,
                PV
            };

            struct StackEntry {
                std::array<Move, 2> killers{};
            };

            bool doMove(Position&, const Move&);
            void doNullMove(Position&);
            void undoMove();
            void undoNullMove();

            void updateHistory(const Piece, const Square, const i32);

            void checkTimeUp();

            template<NodeType nodeType>
            Score search(const Position& pos,
                         Depth           depth,
                         Score           alpha,
                         Score           beta,
                         const i32       ply,
                         const bool      do_null);

            Score quiescencesearch(const Position& pos, Score alpha, Score beta, const i32 ply);

            i32                 id;
            std::atomic_bool    should_stop{false};
            std::vector<u64>    key_history{};
            SearchInfo          info{};
            TranspositionTable& tt;

            size_t nodes{0};

            Move                              pvmove{};
            PieceToHistory                    history{};  // [piece][to]
            std::array<StackEntry, MAX_DEPTH> stack{};
        };

        TranspositionTable      tt{DEFAULT_TT_SIZE_MB};
        std::unique_ptr<Worker> worker;
    };

}
