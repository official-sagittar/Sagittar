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
    else if (argc == 2)
    {
        try {
            std::string cmd = std::string(argv[1]);
            if (cmd == "bench")
            {
                engine.bench();
            }
        }
        catch (const std::exception& e) {
            std::cerr << "Exception: " << e.what() << '\n';
            return 1;
        }
    }
    return 0;
}
