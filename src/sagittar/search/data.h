#pragma once

#include "commons/types.h"
#include "sagittar/core/move.h"
#include "sagittar/core/types.h"
#include "sagittar/search/constants.h"

namespace sagittar {

    namespace search {

        namespace data {

            using namespace commons::types;
            using namespace core::types;

            struct SearcherData {
                u32              history[15][64];  // [piece][to]
                core::move::Move killer_moves[2][constants::MAX_DEPTH];

                SearcherData() { reset(); }

                void reset() {
                    for (u8 p = Piece::NO_PIECE; p <= Piece::BLACK_KING; p++)
                    {
                        for (u8 sq = Square::A1; sq <= Square::H8; sq++)
                        {
                            history[p][sq] = 0;
                        }
                    }
                    for (u8 i = 0; i < constants::MAX_DEPTH; i++)
                    {
                        killer_moves[0][i] = core::move::Move();
                        killer_moves[1][i] = core::move::Move();
                    }
                }
            };

        }

    }

}
