#include "params.h"

namespace sagittar {

    namespace params {

#ifdef EXTERNAL_TUNE
        std::deque<Parameter>& params() {
            static std::deque<Parameter> params;
            return params;
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

        Parameter& addParam(
          std::string name, int value, int min, int max, int step, std::function<void()> callback) {

            params().push_back({name, value, value, min, max, step, callback});
            Parameter& p = params().back();
            return p;
        }
#endif

    }

}
