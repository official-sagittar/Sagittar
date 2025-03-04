#pragma once

#include "pch.h"

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

        PARAM(rfp_depth_max, 3, 2, 5, 1);
        PARAM(rfp_margin, 150, 50, 300, 10);

    }

}
