#include "core/move.h"
#include "core/movegen.h"
#include "core/position.h"
#include "doctest/doctest.h"
#include "pch.h"

using namespace sagittar::core;

TEST_SUITE("Movegen") {

    TEST_CASE("Movegen::movegen_generate_pseudolegal_moves::MOVEGEN_CAPTURES") {
        Position   pos;
        const bool is_valid =
          pos.set_fen("r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq -");
        CHECK(is_valid);

        MoveList moves_list = {};
        movegen_generate_pseudolegal_moves<MovegenType::MOVEGEN_ALL>(&pos, &moves_list);

        size_t captures = 0;
        for (auto [move, score] : moves_list)
        {
            captures += MOVE_IS_CAPTURE(move);
        }

        moves_list = {};
        movegen_generate_pseudolegal_moves<MovegenType::MOVEGEN_CAPTURES>(&pos, &moves_list);

        CHECK(moves_list.size == captures);
    }
}
