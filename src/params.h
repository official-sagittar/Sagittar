#pragma once

#include "pch.h"

namespace sagittar {

    namespace params {

        class Parameters {
           private:
            std::map<std::string, int>   dataInt;
            std::map<std::string, float> dataFloat;

           public:
            void setInt(const std::string key, const int value);
            void setFloat(const std::string key, const float value);

            int   getInt(const std::string key, const int defaultValue) const;
            float getFloat(const std::string key, const float defaultValue) const;
        };

    }

}
