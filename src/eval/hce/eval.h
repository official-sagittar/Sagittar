#pragma once

#include "core/defs.h"
#include "core/position.h"
#include "pch.h"

namespace sagittar {

    namespace eval {

        namespace hce {
            void        eval_init();
            core::Score eval(const core::Position* const pos);
        }

    }

}
