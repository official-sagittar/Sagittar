#include "params.h"


namespace sagittar {

    namespace params {

        void Parameters::setInt(const std::string key, const int value) { dataInt[key] = value; }

        void Parameters::setFloat(const std::string key, const float value) {
            dataFloat[key] = value;
        }

        int Parameters::getInt(const std::string key, const int defaultValue) const {
            auto it = dataInt.find(key);
            if (it != dataInt.end())
            {
                return it->second;
            }
            else
            {
                return defaultValue;
            }
        }

        float Parameters::getFloat(const std::string key, const float defaultValue) const {
            auto it = dataFloat.find(key);
            if (it != dataFloat.end())
            {
                return it->second;
            }
            else
            {
                return defaultValue;
            }
        }

    }

}
