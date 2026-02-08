#pragma once

#include "commons/pch.h"

namespace sagittar::eval::hce::tuner {

    struct TunerSettings {
        bool   compute_k                   = true;
        double K                           = 2.58;
        size_t epochs                      = 10000;
        double beta1                       = 0.9;
        double beta2                       = 0.999;
        double learning_rate_init          = 0.1;
        size_t learning_rate_drop_interval = 1000;
        double learning_rate_drop_ratio    = 0.5;
    };

    void tune(const std::filesystem::path& epd_path, const TunerSettings& settings);

}
