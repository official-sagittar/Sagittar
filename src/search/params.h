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

    class ParameterRegistry {
       public:
        using container_type = std::deque<Parameter>;
        using iterator       = container_type::iterator;
        using const_iterator = container_type::const_iterator;

        static ParameterRegistry& instance();

        Parameter& add(const std::string&           name,
                       int                          value,
                       int                          min,
                       int                          max,
                       int                          step,
                       const std::function<void()>& callback = {});
        bool       set(std::string_view name, int value);
        int        get(std::string_view name) const;

        iterator begin() noexcept { return m_params.begin(); }
        iterator end() noexcept { return m_params.end(); }

        const_iterator begin() const noexcept { return m_params.begin(); }
        const_iterator end() const noexcept { return m_params.end(); }

       private:
        container_type m_params;
    };

    #define PARAM(name, val, min, max, step) \
        inline Parameter& param_##name = \
          ParameterRegistry::instance().add(#name, val, min, max, step); \
        [[nodiscard]] inline int name() { return param_##name.value; }

    #define PARAM_CALLBACK(name, val, min, max, step, callback) \
        inline Parameter& param_##name = \
          ParameterRegistry::instance().add(#name, val, min, max, step, callback); \
        [[nodiscard]] inline int name() { return param_##name.value; }
#else
    #define PARAM(name, val, min, max, step) \
        [[nodiscard]] constexpr int name() { return val; }
    #define PARAM_CALLBACK(name, val, min, max, step, callback) PARAM(name, val, min, max, step)
#endif

    inline double                             lmp_treshold_pct = 0.0;
    inline std::array<std::array<u8, 64>, 64> lmr_r_table_tactical{};  // [move][depth]
    inline std::array<std::array<u8, 64>, 64> lmr_r_table_quiet{};     // [move][depth]
    inline std::array<Score, 8>               futility_margin{};       // [depth]

    void init();
    void updateLMPTresholdPct();
    void updateLMRTable();
    void updateFutilityMargin();

    PARAM(rfp_margin, 51, 50, 1000, 25);

    PARAM_CALLBACK(lmp_treshold, 9, 1, 9, 1, updateLMPTresholdPct);

    PARAM_CALLBACK(lmr_alpha_tactical, 94, 0, 1000, 25, updateLMRTable);
    PARAM_CALLBACK(lmr_beta_tactical, 275, 50, 1000, 25, updateLMRTable);
    PARAM_CALLBACK(lmr_alpha_quiet, 120, 0, 1000, 25, updateLMRTable);
    PARAM_CALLBACK(lmr_beta_quiet, 146, 50, 1000, 25, updateLMRTable);

    PARAM_CALLBACK(futility_margin_c, 9, 5, 500, 5, updateFutilityMargin);
    PARAM_CALLBACK(futility_margin_m, 143, 0, 1000, 10, updateFutilityMargin);

}
