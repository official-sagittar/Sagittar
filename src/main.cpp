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
#ifdef EXTERNAL_TUNE
        else if (cmd == "tune")
        {
            const std::filesystem::path data_path = argv[2];
            engine.tune(data_path);
        }
#endif
    }
    return 0;
}
