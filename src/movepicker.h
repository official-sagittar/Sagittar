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
        constexpr u32 TTMOVE_SCORE        = 30000;
        constexpr u32 MVVLVA_SCORE_OFFSET = 10000;
        constexpr u32 KILLER_0_SCORE      = 9000;
        constexpr u32 KILLER_1_SCORE      = 8000;
        constexpr u32 HISTORY_SCORE_MIN   = 0;
        constexpr u32 HISTORY_SCORE_MAX   = 7000;

        class MovePicker {
           public:
            MovePicker(containers::ArrayList<move::Move>& moves,
                       const board::Board&                board,
                       const move::Move&                  ttmove,
                       const SearcherData&                data,
                       const i32                          ply);

            bool       has_next() const;
            move::Move next();

           private:
            void scoreMoves(const board::Board& board,
                            const move::Move&   ttmove,
                            const SearcherData& data,
                            const i32           ply);

            containers::ArrayList<move::Move>& m_list;
            size_t                             m_index;
        };

    }

}
