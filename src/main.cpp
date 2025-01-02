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
    else if (argc == 2 && std::string(argv[1]) == "bench")
    {
        engine.bench();
    }
    return 0;
}
