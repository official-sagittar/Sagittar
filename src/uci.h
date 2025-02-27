#pragma once

#include "engine.h"
#include "pch.h"

namespace sagittar {

    namespace uci {

        class UCIHandler {
           private:
            Engine& engine;

           private:
            // Standard UCI commands
            void              handleUCI();
            void              handleIsReady();
            void              handleUCINewgame();
            void              handleSetOption(std::string&);
            void              handlePosition(std::string&);
            std::future<void> handleGo(std::string&);
            // Non-standard commands
            void handleDisplay();

           public:
            UCIHandler(Engine& engine);
            void start();
        };

    }

}
