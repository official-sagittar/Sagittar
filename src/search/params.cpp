#include "params.h"

namespace sagittar::search::params {

#ifdef EXTERNAL_TUNE
    std::deque<Parameter>& params() {
        static std::deque<Parameter> params;
        return params;
    }

    Parameter& addParam(
      std::string name, int value, int min, int max, int step, std::function<void()> callback) {

        params().push_back({name, value, value, min, max, step, callback});
        Parameter& p = params().back();
        return p;
    }

    static Parameter* lookup(std::string& name) {
        for (auto& p : params())
        {
            if (p.name == name)
            {
                return &p;
            }
        }

        return nullptr;
    }

    bool set(std::string& name, int value) {
        auto* p = lookup(name);
        if (p != nullptr)
        {
            p->value = value;
            if (p->callback)
            {
                p->callback();
            }
            return true;
        }
        return false;
    }
#endif

    void init() {
        updateLMPTresholdPct();
        updateLMRTable();
        updateFutilityMargin();
    }

    void updateLMPTresholdPct() { lmp_treshold_pct = lmp_treshold / 10.0; }

    void updateLMRTable() {
        const float lmr_alpha_t = static_cast<float>(lmr_alpha_tactical) / 100.0f;
        const float lmr_beta_t  = static_cast<float>(lmr_beta_tactical) / 100.0f;
        const float lmr_alpha_q = static_cast<float>(lmr_alpha_quiet) / 100.0f;
        const float lmr_beta_q  = static_cast<float>(lmr_beta_quiet) / 100.0f;
        u8          r;
        for (u8 depth = 0; depth < 64; depth++)
        {
            for (u8 move = 0; move < 64; move++)
            {
                // Update LMR Tactical table
                auto r_f = lmr_alpha_t + std::log(depth) * std::log(move) / lmr_beta_t;
                auto r_i = std::isfinite(r_f) ? static_cast<int>(r_f) : 0;
                r        = static_cast<u8>(std::min(r_i, depth - 1));
                lmr_r_table_tactical[move][depth] = r;

                // Update LMR Quiet table
                r_f = lmr_alpha_q + std::log(depth) * std::log(move) / lmr_beta_q;
                r_i = std::isfinite(r_f) ? static_cast<int>(r_f) : 0;
                r   = static_cast<u8>(std::min(r_i, depth - 1));
                lmr_r_table_quiet[move][depth] = r;
            }
        }
    }

    void updateFutilityMargin() {
        for (u8 depth = 0; depth < 8; depth++)
        {
            futility_margin[depth] = (futility_margin_m * depth) + futility_margin_c;
        }
    }

}
