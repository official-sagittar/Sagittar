#include "commons/pch.h"
#include "core/bitboard.h"
#include "core/types.h"
#include "doctest/doctest.h"

using namespace sagittar;

TEST_SUITE("BitBoard") {

    TEST_CASE("Ray") {
        BitBoard b      = 0ULL;
        BitBoard should = 0ULL;

        b = ray(Square::A1, Square::H1);
        REQUIRE(b == RANK_1_BB);

        b = ray(Square::H1, Square::H8);
        REQUIRE(b == FILE_H_BB);

        b      = ray(Square::B2, Square::G7);
        should = BB(Square::B2);
        should |= BB(Square::C3);
        should |= BB(Square::D4);
        should |= BB(Square::E5);
        should |= BB(Square::F6);
        should |= BB(Square::G7);
        REQUIRE(b == should);

        b      = ray(Square::D3, Square::D6);
        should = BB(Square::D3);
        should |= BB(Square::D4);
        should |= BB(Square::D5);
        should |= BB(Square::D6);
        REQUIRE(b == should);

        b = ray(Square::D3, Square::E6);
        REQUIRE(b == 0ULL);

        b = ray(Square::G3, Square::B5);
        REQUIRE(b == 0ULL);

        b      = ray(Square::E4, Square::D5);
        should = BB(Square::E4);
        should |= BB(Square::D5);
        REQUIRE(b == should);

        b      = ray(Square::C4, Square::C5);
        should = BB(Square::C4);
        should |= BB(Square::C5);
        REQUIRE(b == should);
    }

    TEST_CASE("Line") {
        BitBoard b      = 0ULL;
        BitBoard should = 0ULL;

        b = line(Square::A1, Square::H1);
        REQUIRE(b == RANK_1_BB);

        b = line(Square::H1, Square::H8);
        REQUIRE(b == FILE_H_BB);

        b      = line(Square::B2, Square::G7);
        should = BB(Square::A1);
        should |= BB(Square::B2);
        should |= BB(Square::C3);
        should |= BB(Square::D4);
        should |= BB(Square::E5);
        should |= BB(Square::F6);
        should |= BB(Square::G7);
        should |= BB(Square::H8);
        REQUIRE(b == should);

        b = line(Square::D3, Square::D6);
        REQUIRE(b == FILE_D_BB);

        b = line(Square::D3, Square::E6);
        REQUIRE(b == 0ULL);

        b = line(Square::G3, Square::B5);
        REQUIRE(b == 0ULL);

        b      = line(Square::E4, Square::D5);
        should = BB(Square::A8);
        should |= BB(Square::B7);
        should |= BB(Square::C6);
        should |= BB(Square::D5);
        should |= BB(Square::E4);
        should |= BB(Square::F3);
        should |= BB(Square::G2);
        should |= BB(Square::H1);
        REQUIRE(b == should);

        b = line(Square::C4, Square::C5);
        REQUIRE(b == FILE_C_BB);
    }

    TEST_CASE("Between") {
        BitBoard b      = 0ULL;
        BitBoard should = 0ULL;

        b      = between(Square::A1, Square::H1);
        should = BB(Square::B1);
        should |= BB(Square::C1);
        should |= BB(Square::D1);
        should |= BB(Square::E1);
        should |= BB(Square::F1);
        should |= BB(Square::G1);
        REQUIRE(b == should);

        b      = between(Square::H1, Square::H8);
        should = BB(Square::H2);
        should |= BB(Square::H3);
        should |= BB(Square::H4);
        should |= BB(Square::H5);
        should |= BB(Square::H6);
        should |= BB(Square::H7);
        REQUIRE(b == should);

        b      = between(Square::B2, Square::G7);
        should = BB(Square::C3);
        should |= BB(Square::D4);
        should |= BB(Square::E5);
        should |= BB(Square::F6);
        REQUIRE(b == should);

        b      = between(Square::D3, Square::D6);
        should = BB(Square::D4);
        should |= BB(Square::D5);
        REQUIRE(b == should);

        b = between(Square::D3, Square::E6);
        REQUIRE(b == 0ULL);

        b = between(Square::G3, Square::B5);
        REQUIRE(b == 0ULL);

        b = between(Square::E4, Square::D5);
        REQUIRE(b == 0ULL);

        b = between(Square::C4, Square::C5);
        REQUIRE(b == 0ULL);
    }
}
