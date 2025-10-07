#include "core/position.h"
#include "pch.h"

int main() {

    sagittar::core::Position pos;
    pos.set_fen("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");
    pos.display();

    return 0;
}
