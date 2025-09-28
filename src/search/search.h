#pragma once

#include "core/defs.h"
#include "core/position.h"
#include "core/tt.h"
#include "pch.h"
#include "search/history.h"
#include "search/types.h"

namespace sagittar {

    namespace search {

        using namespace sagittar::core;

        class Searcher {
           public:
            Searcher();
            void         reset();
            void         reset_for_search();
            void         set_tt_size(const size_t mb);
            void         stop();
            SearchResult start(const Position&                          pos,
                               std::span<uint64_t>                      hash_history,
                               SearchInfo                               info,
                               std::function<void(const SearchResult&)> progress_handler,
                               std::function<void(const SearchResult&)> complete_hander);
            SearchResult
            start(const Position& pos, std::span<uint64_t> hash_history, SearchInfo info);

           private:
            enum class NodeType {
                ROOT,
                NON_ROOT
            };

            struct ThreadData {
                std::vector<uint64_t> hash_history;
                PieceToHistory        history = {};
                size_t                nodes;
                Move                  pv_move;

                ThreadData();
                inline bool do_move(const Position& pos, const Move move, Position& new_pos);
                inline void undo_move();
                inline void update_history(const Piece piece, const Square to, const int depth);
            };

            void         check_timeup(const SearchInfo& info);
            SearchResult search_pos(const Position&                          pos,
                                    ThreadData&                              thread,
                                    const SearchInfo&                        info,
                                    std::function<void(const SearchResult&)> progress_handler,
                                    std::function<void(const SearchResult&)> complete_hander);
            template<Searcher::NodeType nodeType>
            Score search(const Position&   pos,
                         int               depth,
                         Score             alpha,
                         Score             beta,
                         const int         ply,
                         ThreadData&       thread,
                         const SearchInfo& info);
            Score search_quiescence(const Position&   pos,
                                    Score             alpha,
                                    Score             beta,
                                    const int         ply,
                                    ThreadData&       thread,
                                    const SearchInfo& info);

            std::atomic_bool                                      stopped;
            TranspositionTable<TTClient::SEARCH, uint64_t, Score> tt;
        };

    }

}
