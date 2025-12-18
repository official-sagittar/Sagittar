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
    else if (argc == 2)
    {
        std::string cmd = std::string(argv[1]);
        if (cmd == "bench")
        {
            engine.bench();
        }
        else if (cmd == "tune")
        {
            engine.tune();
        }
    }
    return 0;
}
