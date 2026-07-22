#pragma once

#include <cstddef>
#include <filesystem>
#include <thread_pool/thread_pool.h>

namespace sagittar::eval::hce::tuner {

    struct TunerSettings {
        bool   compute_k                   = false;
        bool   retune_from_zero            = false;
        double K                           = 2.38;
        size_t epochs                      = 3000;
        double beta1                       = 0.9;
        double beta2                       = 0.999;
        double learning_rate_init          = 0.01;
        size_t learning_rate_drop_interval = 750;
        double learning_rate_drop_ratio    = 0.5;
        size_t thread_count                = std::thread::hardware_concurrency();
    };

    void tune(const std::filesystem::path& epd_path, const TunerSettings& settings);

}  // namespace sagittar::eval::hce::tuner
