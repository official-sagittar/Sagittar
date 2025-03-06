#pragma once

#include "pch.h"
#include "types.h"

namespace sagittar {

    namespace params {

#ifdef EXTERNAL_TUNE
        struct Parameter {
            std::string           name;
            int                   value;
            int                   default_value;
            int                   min;
            int                   max;
            int                   step;
            std::function<void()> callback;
        };

        std::deque<Parameter>& params();
        Parameter&             addParam(std::string           name,
                                        int                   value,
                                        int                   min,
                                        int                   max,
                                        int                   step,
                                        std::function<void()> callback = std::function<void()>());
        bool                   set(std::string& name, int value);

    #define PARAM(name, val, min, max, step) \
        inline Parameter& param_##name = addParam(#name, val, min, max, step); \
        inline const int& name         = param_##name.value;

    #define PARAM_CALLBACK(name, val, min, max, step, callback) \
        inline Parameter& param_##name = addParam(#name, val, min, max, step, callback); \
        inline const int& name         = param_##name.value;
#else
    #define PARAM(name, val, min, max, step) constexpr int name = val;

    #define PARAM_CALLBACK(name, val, min, max, step, callback) PARAM(name, val, min, max, step)
#endif

        inline double lmp_treshold_pct;
        inline u8     lmr_r_table_tactical[64][64];  // [move][depth]
        inline u8     lmr_r_table_quiet[64][64];     // [move][depth]

        void init();
        void updateLMPTresholdPct();
        void updateLMRTable();

        PARAM(rfp_depth_max, 3, 1, 5, 1);
        PARAM(rfp_margin, 150, 50, 300, 25);

        PARAM(nmp_depth_min, 3, 1, 5, 1);

        PARAM(lmp_depth_max, 2, 1, 5, 1);
        PARAM_CALLBACK(lmp_treshold, 6, 2, 8, 1, updateLMPTresholdPct);

        PARAM(lmr_depth_min, 3, 1, 5, 1);
        PARAM(lmr_movesearched_min, 4, 1, 10, 1);
        PARAM_CALLBACK(lmr_alpha_tactical, 0, 0, 300, 50, updateLMRTable);
        PARAM_CALLBACK(lmr_beta_tactical, 275, 50, 500, 25, updateLMRTable);
        PARAM_CALLBACK(lmr_alpha_quiet, 100, 0, 300, 50, updateLMRTable);
        PARAM_CALLBACK(lmr_beta_quiet, 150, 50, 500, 25, updateLMRTable);
    }

}
