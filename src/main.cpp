#include "datagen/datagen.h"
#include "engine.h"
#include "pch.h"
#include "uci.h"

int main(int argc, char* argv[]) {
    sagittar::Engine engine;
    if (argc == 1)
    {
        sagittar::uci::UCIHandler ucihandler(engine);
        ucihandler.start();
    }
    else if (argc >= 2)
    {
        std::string cmd = std::string(argv[1]);

        if (cmd == "bench")
        {
            engine.bench();
        }
        else if (cmd.rfind("genfens", 0) == 0)
        {
            sagittar::datagen::genfens(cmd);

            if (std::string(argv[2]) == "quit")
            {
                return 0;
            }
        }
    }
    return 0;
}
