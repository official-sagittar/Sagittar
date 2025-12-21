#include "commons/pch.h"
#include "comms/uci.h"
#include "engine.h"

int main(int argc, char* argv[]) {
    sagittar::Engine engine;
    if (argc == 1)
    {
        sagittar::comms::uci::UCIHandler ucihandler(engine);
        ucihandler.start();
    }
    else if (argc >= 2)
    {
        std::string cmd = std::string(argv[1]);
        if (cmd == "bench")
        {
            engine.bench();
        }
        else if (cmd == "tune")
        {
            if (argc >= 3)
            {
                const std::filesystem::path data_path = argv[2];
                engine.tune(data_path);
            }
        }
    }
    return 0;
}
