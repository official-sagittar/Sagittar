#pragma once

#include "commons/types.h"

namespace sagittar {

    namespace eval {

        namespace hce {

            namespace types {

                using namespace commons::types;

                enum GamePhase {
                    MG,
                    EG
                };

                constexpr i32 S(const i32 mg, const i32 eg) {
                    return static_cast<i32>(static_cast<u32>(eg) << 16) + mg;
                }

                constexpr i32 mg_score(const i32 score) { return static_cast<i16>(score); }

                constexpr i32 eg_score(const i32 score) {
                    return static_cast<i16>((score + 0x8000) >> 16);
                }

            }

        }

    }

}
