#pragma once

#include "commons/pch.h"

namespace sagittar::eval::hce::tuner {

    struct TunerSettings {
        bool   compute_k                   = true;
        bool   retune_from_zero            = false;
        double K                           = 2.9;
        size_t epochs                      = 10000;
        double beta1                       = 0.9;
        double beta2                       = 0.999;
        double learning_rate_init          = 0.15;
        size_t learning_rate_drop_interval = 2000;
        double learning_rate_drop_ratio    = 0.75;
    };

    void tune(const std::filesystem::path& epd_path, const TunerSettings& settings);

}
