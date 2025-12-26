#pragma once

#include "commons/pch.h"

namespace sagittar::eval::hce::tuner {

    struct TunerSettings {
        bool   compute_k           = true;
        double K                   = 2.0;
        size_t epochs              = 5000;
        double beta1               = 0.9;
        double beta2               = 0.999;
        double learning_rate_init  = 0.1;
        double learning_rate_decay = 0.001;
    };

    void tune(const std::filesystem::path& epd_path, const TunerSettings& settings);

}
