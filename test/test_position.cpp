#include "core/position.h"
#include "doctest/doctest.h"
#include "pch.h"

using namespace sagittar::core;

TEST_SUITE("Position") {

    TEST_CASE("Position Pinned") {
        Position pos;

        bool is_valid = pos.set_fen("4k3/8/8/b7/8/8/3P4/4K3 w - - 0 1");
        CHECK(is_valid);
        REQUIRE(__builtin_ctzll(pos.pinned) == D2);

        is_valid = pos.set_fen("4k3/8/8/8/8/8/r2PK3/8 w - - 0 1");
        CHECK(is_valid);
        REQUIRE(__builtin_ctzll(pos.pinned) == D2);
    }

    TEST_CASE("Position::is_repeated") {
        Position              pos;
        std::vector<uint64_t> hash_history;

        bool is_valid = pos.set_fen("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");
        CHECK(is_valid);

        Position new_pos = pos;

        is_valid = pos.do_move("g1f3", new_pos);
        CHECK(is_valid);
        if (is_valid)
        {
            hash_history.push_back(pos.hash);
            REQUIRE_FALSE(new_pos.is_repeated(hash_history));
            pos = new_pos;
        }

        is_valid = pos.do_move("g8f6", new_pos);
        CHECK(is_valid);
        if (is_valid)
        {
            hash_history.push_back(pos.hash);
            REQUIRE_FALSE(new_pos.is_repeated(hash_history));
            pos = new_pos;
        }

        is_valid = pos.do_move("f3g1", new_pos);
        CHECK(is_valid);
        if (is_valid)
        {
            hash_history.push_back(pos.hash);
            REQUIRE_FALSE(new_pos.is_repeated(hash_history));
            pos = new_pos;
        }

        is_valid = pos.do_move("f6g8", new_pos);
        CHECK(is_valid);
        if (is_valid)
        {
            hash_history.push_back(pos.hash);
            REQUIRE(new_pos.is_repeated(hash_history));
            pos = new_pos;
        }

        is_valid = pos.do_move("g1f3", new_pos);
        CHECK(is_valid);
        if (is_valid)
        {
            hash_history.push_back(pos.hash);
            REQUIRE(new_pos.is_repeated(hash_history));
            pos = new_pos;
        }

        is_valid = pos.do_move("g8f6", new_pos);
        CHECK(is_valid);
        if (is_valid)
        {
            hash_history.push_back(pos.hash);
            REQUIRE(new_pos.is_repeated(hash_history));
            pos = new_pos;
        }

        is_valid = pos.do_move("f3g1", new_pos);
        CHECK(is_valid);
        if (is_valid)
        {
            hash_history.push_back(pos.hash);
            REQUIRE(new_pos.is_repeated(hash_history));
            pos = new_pos;
        }

        is_valid = pos.do_move("f6g8", new_pos);
        CHECK(is_valid);
        if (is_valid)
        {
            hash_history.push_back(pos.hash);
            REQUIRE(new_pos.is_repeated(hash_history));
            pos = new_pos;
        }
    }
}
