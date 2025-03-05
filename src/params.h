#pragma once

#include "pch.h"

namespace sagittar {

    namespace params {

#ifdef EXTERNAL_TUNE
        struct Parameter {
            std::string name;
            int         value;
            int         default_value;
            int         min;
            int         max;
            int         step;
        };

        std::deque<Parameter>& params();
        Parameter&             addParam(std::string name, int value, int min, int max, int step);
        bool                   set(std::string& name, int value);

    #define PARAM(name, val, min, max, step) \
        inline Parameter& param_##name = addParam(#name, val, min, max, step); \
        inline const int& name         = param_##name.value;
#else
    #define PARAM(name, val, min, max, step) constexpr int name = val;
#endif

        PARAM(rfp_depth_max, 3, 1, 8, 1);
        PARAM(rfp_margin, 150, 50, 300, 10);

        PARAM(nmp_depth_min, 3, 1, 8, 1);

        PARAM(lmp_depth_max, 2, 1, 8, 1);
        PARAM(lmp_treshold, 6, 1, 8, 1);

        PARAM(lmr_depth_min, 3, 1, 8, 1);
        PARAM(lmr_movesearched_min, 4, 1, 8, 1);
        PARAM(lmr_alpha_tactical, 0, 0, 500, 10);
        PARAM(lmr_beta_tactical, 275, 50, 500, 5);
        PARAM(lmr_alpha_quiet, 100, 0, 500, 10);
        PARAM(lmr_beta_quiet, 150, 50, 500, 5);
    }

}
