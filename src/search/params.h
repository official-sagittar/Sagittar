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
        using ContainerType = std::deque<Parameter>;
        using Iterator      = ContainerType::iterator;
        using ConstIterator = ContainerType::const_iterator;

        static ParameterRegistry& instance();

        Parameter& add(const std::string&           name,
                       int                          value,
                       int                          min,
                       int                          max,
                       int                          step,
                       const std::function<void()>& callback = {});
        bool       set(std::string_view name, int value);
        int        get(std::string_view name) const;

        Iterator begin() noexcept { return m_params.begin(); }
        Iterator end() noexcept { return m_params.end(); }

        ConstIterator begin() const noexcept { return m_params.begin(); }
        ConstIterator end() const noexcept { return m_params.end(); }

       private:
        ContainerType m_params;
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

    inline std::array<std::array<U8, 64>, 64> g_lmr_r_table_tactical{};  // [move][depth]
    inline std::array<std::array<U8, 64>, 64> g_lmr_r_table_quiet{};     // [move][depth]

    void init();
    void updateLMRTable();

    PARAM(rfpMargin, 51, 50, 1000, 25);

    PARAM(lmpBase, 12, 1, 20, 1);
    PARAM(lmpDepthScale, 100, 25, 200, 10);

    PARAM_CALLBACK(lmrAlphaTactical, 94, 0, 1000, 25, updateLMRTable);
    PARAM_CALLBACK(lmrBetaTactical, 275, 50, 1000, 25, updateLMRTable);
    PARAM_CALLBACK(lmrAlphaQuiet, 120, 0, 1000, 25, updateLMRTable);
    PARAM_CALLBACK(lmrBetaQuiet, 146, 50, 1000, 25, updateLMRTable);

    PARAM(futilityMarginC, 9, 5, 500, 5);
    PARAM(futilityMarginM, 143, 0, 1000, 10);

}
