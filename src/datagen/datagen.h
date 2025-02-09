#pragma once

#include "engine.h"
#include "pch.h"
#include "types.h"

namespace sagittar {

    namespace datagen {

        class DataGenerator {
           private:
            Engine&     engine;
            u32         N;
            u64         S;
            std::string book;
            u32         generated;

           private:
            void selfplay();
            void start();

           public:
            DataGenerator(Engine& engine);
            void genfens(std::string& input);
        };

    }

}
