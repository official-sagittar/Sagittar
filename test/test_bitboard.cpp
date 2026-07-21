#include "commons/pch.h"
#include "core/bitboard.h"
#include "core/types.h"
#include "doctest/doctest.h"

using namespace sagittar;

TEST_SUITE("BitBoard") {

    TEST_CASE("Ray") {
        BitBoard b{};
        BitBoard should{};

        b = ray(Square{Square::Raw::A1}, Square{Square::Raw::H1});
        REQUIRE(b == RANK_1_BB);

        b = ray(Square{Square::Raw::H1}, Square{Square::Raw::H8});
        REQUIRE(b == FILE_H_BB);

        b      = ray(Square{Square::Raw::B2}, Square{Square::Raw::G7});
        should = BB(Square{Square::Raw::B2});
        should |= BB(Square{Square::Raw::C3});
        should |= BB(Square{Square::Raw::D4});
        should |= BB(Square{Square::Raw::E5});
        should |= BB(Square{Square::Raw::F6});
        should |= BB(Square{Square::Raw::G7});
        REQUIRE(b == should);

        b      = ray(Square{Square::Raw::D3}, Square{Square::Raw::D6});
        should = BB(Square{Square::Raw::D3});
        should |= BB(Square{Square::Raw::D4});
        should |= BB(Square{Square::Raw::D5});
        should |= BB(Square{Square::Raw::D6});
        REQUIRE(b == should);

        b = ray(Square{Square::Raw::D3}, Square{Square::Raw::E6});
        REQUIRE(b.is_empty());

        b = ray(Square{Square::Raw::G3}, Square{Square::Raw::B5});
        REQUIRE(b.is_empty());

        b      = ray(Square{Square::Raw::E4}, Square{Square::Raw::D5});
        should = BB(Square{Square::Raw::E4});
        should |= BB(Square{Square::Raw::D5});
        REQUIRE(b == should);

        b      = ray(Square{Square::Raw::C4}, Square{Square::Raw::C5});
        should = BB(Square{Square::Raw::C4});
        should |= BB(Square{Square::Raw::C5});
        REQUIRE(b == should);
    }

    TEST_CASE("Line") {
        BitBoard b      = 0ULL;
        BitBoard should = 0ULL;

        b = line(Square{Square::Raw::A1}, Square{Square::Raw::H1});
        REQUIRE(b == RANK_1_BB);

        b = line(Square{Square::Raw::H1}, Square{Square::Raw::H8});
        REQUIRE(b == FILE_H_BB);

        b      = line(Square{Square::Raw::B2}, Square{Square::Raw::G7});
        should = BB(Square{Square::Raw::A1});
        should |= BB(Square{Square::Raw::B2});
        should |= BB(Square{Square::Raw::C3});
        should |= BB(Square{Square::Raw::D4});
        should |= BB(Square{Square::Raw::E5});
        should |= BB(Square{Square::Raw::F6});
        should |= BB(Square{Square::Raw::G7});
        should |= BB(Square{Square::Raw::H8});
        REQUIRE(b == should);

        b = line(Square{Square::Raw::D3}, Square{Square::Raw::D6});
        REQUIRE(b == FILE_D_BB);

        b = line(Square{Square::Raw::D3}, Square{Square::Raw::E6});
        REQUIRE(b.is_empty());

        b = line(Square{Square::Raw::G3}, Square{Square::Raw::B5});
        REQUIRE(b.is_empty());

        b      = line(Square{Square::Raw::E4}, Square{Square::Raw::D5});
        should = BB(Square{Square::Raw::A8});
        should |= BB(Square{Square::Raw::B7});
        should |= BB(Square{Square::Raw::C6});
        should |= BB(Square{Square::Raw::D5});
        should |= BB(Square{Square::Raw::E4});
        should |= BB(Square{Square::Raw::F3});
        should |= BB(Square{Square::Raw::G2});
        should |= BB(Square{Square::Raw::H1});
        REQUIRE(b == should);

        b = line(Square{Square::Raw::C4}, Square{Square::Raw::C5});
        REQUIRE(b == FILE_C_BB);
    }

    TEST_CASE("Between") {
        BitBoard b      = 0ULL;
        BitBoard should = 0ULL;

        b      = between(Square{Square::Raw::A1}, Square{Square::Raw::H1});
        should = BB(Square{Square::Raw::B1});
        should |= BB(Square{Square::Raw::C1});
        should |= BB(Square{Square::Raw::D1});
        should |= BB(Square{Square::Raw::E1});
        should |= BB(Square{Square::Raw::F1});
        should |= BB(Square{Square::Raw::G1});
        REQUIRE(b == should);

        b      = between(Square{Square::Raw::H1}, Square{Square::Raw::H8});
        should = BB(Square{Square::Raw::H2});
        should |= BB(Square{Square::Raw::H3});
        should |= BB(Square{Square::Raw::H4});
        should |= BB(Square{Square::Raw::H5});
        should |= BB(Square{Square::Raw::H6});
        should |= BB(Square{Square::Raw::H7});
        REQUIRE(b == should);

        b      = between(Square{Square::Raw::B2}, Square{Square::Raw::G7});
        should = BB(Square{Square::Raw::C3});
        should |= BB(Square{Square::Raw::D4});
        should |= BB(Square{Square::Raw::E5});
        should |= BB(Square{Square::Raw::F6});
        REQUIRE(b == should);

        b      = between(Square{Square::Raw::D3}, Square{Square::Raw::D6});
        should = BB(Square{Square::Raw::D4});
        should |= BB(Square{Square::Raw::D5});
        REQUIRE(b == should);

        b = between(Square{Square::Raw::D3}, Square{Square::Raw::E6});
        REQUIRE(b.is_empty());

        b = between(Square{Square::Raw::G3}, Square{Square::Raw::B5});
        REQUIRE(b.is_empty());

        b = between(Square{Square::Raw::E4}, Square{Square::Raw::D5});
        REQUIRE(b.is_empty());

        b = between(Square{Square::Raw::C4}, Square{Square::Raw::C5});
        REQUIRE(b.is_empty());
    }
}
