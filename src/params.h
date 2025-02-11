#pragma once

#include "containers.h"
#include "pch.h"

namespace sagittar {

    namespace parameters {

        class ParameterStore {
           private:
            containers::VariantMap<int, float> params;

           public:
            template<typename T>
            void set(const std::string& key, const T value) {
                params.insert<T>(key, value);
            }

            template<typename T>
            T get(const std::string& key, const T defaultValue) const {
                return params.get(key, defaultValue);
            }
        };

    }

}
