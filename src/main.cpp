#include "comms/uci.h"
#include "engine.h"
#include "pch.h"

int main(int argc, char* argv[]) {
    sagittar::Engine e;

    if (argc == 1)
    {
        sagittar::comms::uci::UCIHandler ucihandler(e);
        ucihandler.start();
    }
    else if (argc == 2)
    {
        std::string cmd = std::string(argv[1]);
        if (cmd == "bench")
        {
            e.bench();
        }
    }

    return 0;
}
