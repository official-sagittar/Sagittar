#pragma once

#include "core/defs.h"
#include "core/position.h"
#include "pch.h"

namespace sagittar {

    namespace eval {

        using namespace sagittar::core;

        namespace hce {
            void        eval_init();
            core::Score eval(const Position* const pos);
        }

    }

}
