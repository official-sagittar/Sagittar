#include "params.h"

namespace sagittar::search::params {

#ifdef EXTERNAL_TUNE

    ParameterRegistry& ParameterRegistry::instance() {
        static ParameterRegistry reg;
        return reg;
    }

    Parameter& ParameterRegistry::add(const std::string&           name,
                                      int                          value,
                                      int                          min,
                                      int                          max,
                                      int                          step,
                                      const std::function<void()>& callback) {
        m_params.push_back({name, value, value, min, max, step, callback});
        return m_params.back();
    }

    bool ParameterRegistry::set(std::string_view name, int value) {
        for (auto& p : m_params)
        {
            if (p.name == name)
            {
                p.value = std::clamp(value, p.min, p.max);
                if (p.callback)
                    p.callback();
                return true;
            }
        }
        return false;
    }

    int ParameterRegistry::get(std::string_view name) const {
        for (auto& p : m_params)
            if (p.name == name)
                return p.value;
        return 0;
    }

#endif

    void updateLMPTresholdPct() { lmp_treshold_pct = lmp_treshold() / 10.0; }

    void updateLMRTable() {
        const float lmr_alpha_t = static_cast<float>(lmr_alpha_tactical()) / 100.0f;
        const float lmr_beta_t  = static_cast<float>(lmr_beta_tactical()) / 100.0f;
        const float lmr_alpha_q = static_cast<float>(lmr_alpha_quiet()) / 100.0f;
        const float lmr_beta_q  = static_cast<float>(lmr_beta_quiet()) / 100.0f;
        u8          r;
        for (u8 depth = 0; depth < 64; depth++)
        {
            for (u8 move = 0; move < 64; move++)
            {
                const int max_r = depth > 0 ? depth - 1 : 0;

                // Update LMR Tactical table
                float r_f = 0.0;
                if (depth > 0 && move > 0)
                {
                    r_f = lmr_alpha_t + std::log(depth) * std::log(move) / lmr_beta_t;
                }
                int r_i = std::isfinite(r_f) ? static_cast<int>(r_f) : 0;
                r       = static_cast<u8>(std::clamp(r_i, 0, max_r));

                lmr_r_table_tactical[move][depth] = r;

                // Update LMR Quiet table
                r_f = 0.0;
                if (depth > 0 && move > 0)
                {
                    r_f = lmr_alpha_q + std::log(depth) * std::log(move) / lmr_beta_q;
                }
                r_i = std::isfinite(r_f) ? static_cast<int>(r_f) : 0;
                r   = static_cast<u8>(std::clamp(r_i, 0, max_r));

                lmr_r_table_quiet[move][depth] = r;
            }
        }
    }

    void updateFutilityMargin() {
        for (u8 depth = 0; depth < 8; depth++)
        {
            futility_margin[depth] = (futility_margin_m() * depth) + futility_margin_c();
        }
    }

    void init() {
        updateLMPTresholdPct();
        updateLMRTable();
        updateFutilityMargin();
    }

}
