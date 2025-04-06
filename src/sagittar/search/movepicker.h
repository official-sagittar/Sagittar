#pragma once

#include "commons/containers.h"
#include "commons/pch.h"
#include "commons/types.h"
#include "sagittar/core/board.h"
#include "sagittar/core/move.h"
#include "sagittar/core/types.h"
#include "sagittar/search/data.h"
#include "sagittar/search/search.h"

namespace sagittar {

    namespace search {

        namespace movepicker {

            using namespace commons::types;
            using namespace core::types;

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
            constexpr u32 PVMOVE_SCORE        = 40000;
            constexpr u32 TTMOVE_SCORE        = 30000;
            constexpr u32 MVVLVA_SCORE_OFFSET = 10000;
            constexpr u32 KILLER_0_SCORE      = 9000;
            constexpr u32 KILLER_1_SCORE      = 8000;
            constexpr u32 HISTORY_SCORE_MIN   = 0;
            constexpr u32 HISTORY_SCORE_MAX   = 7000;

            void scoreMoves(commons::containers::ArrayList<core::move::Move>* moves,
                            const core::board::Board&                         board,
                            const core::move::Move&                           pvmove,
                            const core::move::Move&                           ttmove,
                            const data::SearcherData&                         data,
                            const i32                                         ply);
            void sortMoves(commons::containers::ArrayList<core::move::Move>* moves, const u8 index);

        }

    }

}
