#pragma once

#include "board.h"
#include "containers.h"
#include "move.h"
#include "pch.h"
#include "search.h"
#include "types.h"

namespace sagittar {

    namespace search {

        // clang-format off
        /*
            (Victims) Pawn   Knight Bishop Rook   Queen  King
        (Attackers)
        Pawn          105    205    305    405    505    605
        Knight        104    204    304    404    504    604
        Bishop        103    203    303    403    503    603
        Rook          102    202    302    402    502    602
        Queen         101    201    301    401    501    601
        King          100    200    300    400    500    600
        */
        const u32 MVV_LVA_TABLE[36] = {
            105, 205, 305, 405, 505, 605,
            104, 204, 304, 404, 504, 604,
            103, 203, 303, 403, 503, 603,
            102, 202, 302, 402, 502, 602,
            101, 201, 301, 401, 501, 601,
            100, 200, 300, 400, 500, 600
        };
        // clang-format on

        constexpr u8 mvvlvaIdx(const PieceType attacker, const PieceType victim) {
            return ((attacker - 1) * 6) + (victim - 1);
        }

        constexpr u32 HISTORY_SCORE_MIN = 0;
        constexpr u32 HISTORY_SCORE_MAX = 7000;

        enum class MovePickerPhase {
            TT_MOVE,
            CAPTURES,
            KILLERS,
            QUIETS,
            DONE
        };

        class MovePicker {
           public:
            MovePicker() = delete;
            explicit MovePicker(const containers::ArrayList<move::Move>& moves,
                                const board::Board&                      board,
                                const move::Move&                        ttmove,
                                const SearcherData&                      data,
                                const i32                                ply);
            MovePicker(const MovePicker&)                = delete;
            MovePicker(MovePicker&&) noexcept            = delete;
            MovePicker& operator=(const MovePicker&)     = delete;
            MovePicker& operator=(MovePicker&&) noexcept = delete;
            void*       operator new(std::size_t)        = delete;
            void*       operator new[](std::size_t)      = delete;
            ~MovePicker() noexcept                       = default;

            MovePickerPhase phase() const;
            bool            hasNext() const;
            move::Move      next();

           private:
            struct ScoredMove final {
                move::Move move;
                u32        score;

                ScoredMove() noexcept :
                    move(),
                    score(0) {}

                ScoredMove(const move::Move& m, u32 s) noexcept :
                    move(m),
                    score(s) {}

                ScoredMove(const ScoredMove& other) noexcept :
                    move(other.move),
                    score(other.score) {}

                ScoredMove(ScoredMove&& other) noexcept :
                    move(std::move(other.move)),
                    score(other.score) {}

                ScoredMove& operator=(const ScoredMove& other) noexcept {
                    if (this != &other)
                    {
                        move  = other.move;
                        score = other.score;
                    }
                    return *this;
                }

                ScoredMove& operator=(ScoredMove&& other) noexcept {
                    if (this != &other)
                    {
                        move  = std::move(other.move);
                        score = other.score;
                    }
                    return *this;
                }

                ~ScoredMove() noexcept = default;
            };

            void processMoves(const containers::ArrayList<move::Move>& moves,
                              const board::Board&                      board,
                              const move::Move&                        ttmove,
                              const SearcherData&                      data,
                              const i32                                ply);

            move::Move                m_tt_move{};
            std::vector<ScoredMove>   m_captures{};
            std::vector<ScoredMove>   m_quiets{};
            std::array<move::Move, 2> m_killers{};
            MovePickerPhase           m_phase{MovePickerPhase::TT_MOVE};
            size_t                    m_moves_count{0};
            size_t                    m_index{0};
            size_t                    m_index_captures{0};
            size_t                    m_index_killers{0};
            size_t                    m_index_quiets{0};
        };

    }

}
