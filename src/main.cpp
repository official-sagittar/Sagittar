#include "engine.h"
#include "pch.h"

int main() {
    sagittar::Engine e;
    e.set_tt_size_mb(64);
    const bool is_valid =
      e.set_fen("r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 1");
    if (is_valid)
    {
        e.display_position();
        e.perft(5);
    }
    return 0;
}
