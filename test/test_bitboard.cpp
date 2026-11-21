#include "commons/pch.h"
#include "core/bitboard.h"
#include "core/types.h"
#include "doctest/doctest.h"

using namespace sagittar;

TEST_SUITE("BitBoard") {

    TEST_CASE("Ray") {
        BitBoard b{};
        BitBoard should{};

        b = ray(Square::A1, Square::H1);
        REQUIRE(b == k_RANK_1_BB);

        b = ray(Square::H1, Square::H8);
        REQUIRE(b == k_FILE_H_BB);

        b      = ray(Square::B2, Square::G7);
        should = bb(Square::B2);
        should |= bb(Square::C3);
        should |= bb(Square::D4);
        should |= bb(Square::E5);
        should |= bb(Square::F6);
        should |= bb(Square::G7);
        REQUIRE(b == should);

        b      = ray(Square::D3, Square::D6);
        should = bb(Square::D3);
        should |= bb(Square::D4);
        should |= bb(Square::D5);
        should |= bb(Square::D6);
        REQUIRE(b == should);

        b = ray(Square::D3, Square::E6);
        REQUIRE(b.isEmpty());

        b = ray(Square::G3, Square::B5);
        REQUIRE(b.isEmpty());

        b      = ray(Square::E4, Square::D5);
        should = bb(Square::E4);
        should |= bb(Square::D5);
        REQUIRE(b == should);

        b      = ray(Square::C4, Square::C5);
        should = bb(Square::C4);
        should |= bb(Square::C5);
        REQUIRE(b == should);
    }

    TEST_CASE("Line") {
        BitBoard b      = 0ULL;
        BitBoard should = 0ULL;

        b = line(Square::A1, Square::H1);
        REQUIRE(b == k_RANK_1_BB);

        b = line(Square::H1, Square::H8);
        REQUIRE(b == k_FILE_H_BB);

        b      = line(Square::B2, Square::G7);
        should = bb(Square::A1);
        should |= bb(Square::B2);
        should |= bb(Square::C3);
        should |= bb(Square::D4);
        should |= bb(Square::E5);
        should |= bb(Square::F6);
        should |= bb(Square::G7);
        should |= bb(Square::H8);
        REQUIRE(b == should);

        b = line(Square::D3, Square::D6);
        REQUIRE(b == k_FILE_D_BB);

        b = line(Square::D3, Square::E6);
        REQUIRE(b.isEmpty());

        b = line(Square::G3, Square::B5);
        REQUIRE(b.isEmpty());

        b      = line(Square::E4, Square::D5);
        should = bb(Square::A8);
        should |= bb(Square::B7);
        should |= bb(Square::C6);
        should |= bb(Square::D5);
        should |= bb(Square::E4);
        should |= bb(Square::F3);
        should |= bb(Square::G2);
        should |= bb(Square::H1);
        REQUIRE(b == should);

        b = line(Square::C4, Square::C5);
        REQUIRE(b == k_FILE_C_BB);
    }

    TEST_CASE("Between") {
        BitBoard b      = 0ULL;
        BitBoard should = 0ULL;

        b      = between(Square::A1, Square::H1);
        should = bb(Square::B1);
        should |= bb(Square::C1);
        should |= bb(Square::D1);
        should |= bb(Square::E1);
        should |= bb(Square::F1);
        should |= bb(Square::G1);
        REQUIRE(b == should);

        b      = between(Square::H1, Square::H8);
        should = bb(Square::H2);
        should |= bb(Square::H3);
        should |= bb(Square::H4);
        should |= bb(Square::H5);
        should |= bb(Square::H6);
        should |= bb(Square::H7);
        REQUIRE(b == should);

        b      = between(Square::B2, Square::G7);
        should = bb(Square::C3);
        should |= bb(Square::D4);
        should |= bb(Square::E5);
        should |= bb(Square::F6);
        REQUIRE(b == should);

        b      = between(Square::D3, Square::D6);
        should = bb(Square::D4);
        should |= bb(Square::D5);
        REQUIRE(b == should);

        b = between(Square::D3, Square::E6);
        REQUIRE(b.isEmpty());

        b = between(Square::G3, Square::B5);
        REQUIRE(b.isEmpty());

        b = between(Square::E4, Square::D5);
        REQUIRE(b.isEmpty());

        b = between(Square::C4, Square::C5);
        REQUIRE(b.isEmpty());
    }
}
