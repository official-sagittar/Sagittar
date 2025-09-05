#include "core/defs.h"
#include "core/movegen.h"
#include "core/perft.h"
#include "core/position.h"
#include "core/tt.h"
#include "pch.h"

using namespace sagittar::core;

int main() {
    position_init();
    movegen_init();
    Position   pos;
    const bool is_valid =
      pos.set_fen("r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 1");
    if (is_valid)
    {
        pos.display();
        TranspositionTable tt;
        tt.resize(64);
        using clock = std::chrono::high_resolution_clock;
        auto start  = clock::now();
        auto nodes  = divide(&pos, 5, &tt);
        auto end    = clock::now();
        auto elapsed_ms =
          std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
        std::cout << "Elapsed = " << elapsed_ms
                  << " ms\nNPS = " << (double) (nodes / (elapsed_ms / 1000)) / 1000000 << " million"
                  << std::endl;
    }
    return 0;
}
