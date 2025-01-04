#pragma once

#include "engine.h"
#include "pch.h"
#include "search.h"

namespace sagittar {

    namespace uci {

        class UCIHandler {
           private:
            Engine& engine;

           private:
            void              handleUCI();
            void              handleIsReady();
            void              handleUCINewgame();
            void              handleDisplay();
            void              handlePosition(std::string&);
            std::future<void> handleGo(std::string&);

           public:
            UCIHandler(Engine& engine);
            void start();
        };

    }

}
