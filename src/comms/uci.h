#pragma once

#include "engine.h"

#include <future>
#include <string>

namespace sagittar::comms::uci {

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
#ifdef EXTERNAL_TUNE
        void handleDisplayParams();
#endif

       public:
        UCIHandler(Engine& engine);
        void start();
    };

}  // namespace sagittar::comms::uci
