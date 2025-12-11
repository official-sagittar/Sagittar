#pragma once

#include "commons/pch.h"
#include "core/types.h"

namespace sagittar::search::params {

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
    inline Score  futility_margin[8];            // [depth]

    void init();
    void updateLMPTresholdPct();
    void updateLMRTable();
    void updateFutilityMargin();

    PARAM(rfp_margin, 50, 50, 1000, 25);

    PARAM_CALLBACK(lmp_treshold, 8, 1, 9, 1, updateLMPTresholdPct);

    PARAM_CALLBACK(lmr_alpha_tactical, 68, 0, 1000, 25, updateLMRTable);
    PARAM_CALLBACK(lmr_beta_tactical, 265, 50, 1000, 25, updateLMRTable);
    PARAM_CALLBACK(lmr_alpha_quiet, 131, 0, 1000, 25, updateLMRTable);
    PARAM_CALLBACK(lmr_beta_quiet, 137, 50, 1000, 25, updateLMRTable);

    PARAM_CALLBACK(futility_margin_c, 10, 5, 500, 5, updateFutilityMargin);
    PARAM_CALLBACK(futility_margin_m, 160, 0, 1000, 10, updateFutilityMargin);

}
