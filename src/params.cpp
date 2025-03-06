#include "params.h"

namespace sagittar {

    namespace params {

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
            auto* p = params::lookup(name);
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
            update_lmp_treshold_pct();
            update_lmr_table();
        }

        void update_lmp_treshold_pct() { lmp_treshold_pct = lmp_treshold / 10.0; }

        void update_lmr_table() {
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
                    r                                 = static_cast<u8>(std::min(
                      static_cast<int>(lmr_alpha_t + std::log(depth) * std::log(move) / lmr_beta_t),
                      depth - 1));
                    lmr_r_table_tactical[move][depth] = r;

                    // Update LMR Quiet table
                    r                              = static_cast<u8>(std::min(
                      static_cast<int>(lmr_alpha_q + std::log(depth) * std::log(move) / lmr_beta_q),
                      depth - 1));
                    lmr_r_table_quiet[move][depth] = r;
                }
            }
        }

    }

}
