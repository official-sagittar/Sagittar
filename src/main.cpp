#include "comms/uci.h"
#include "engine.h"
#include "pch.h"

int main() {
    sagittar::Engine e;

    sagittar::comms::uci::UCIHandler ucihandler(e);
    ucihandler.start();

    return 0;
}
