#pragma once

#include "engine.h"
#include "pch.h"

namespace sagittar {

    namespace comms {

        namespace uci {

            class UCIHandler {
               public:
                UCIHandler(Engine& engine);
                void start();

               private:
                // Standard UCI commands
                void              handle_uci();
                void              handle_isready();
                void              handle_ucinewgame();
                void              handle_setoption(std::string&);
                void              handle_position(std::string&);
                std::future<void> handle_go(std::string&);
                // Non-standard commands
                void handle_display();

                Engine& engine;
            };

        }

    }

}
