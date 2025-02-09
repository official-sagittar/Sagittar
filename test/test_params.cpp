#include "doctest/doctest.h"
#include "params.h"
#include "pch.h"

using namespace sagittar;

TEST_SUITE("Params") {

    TEST_CASE("Params::set and Params::get") {
        const float EPSILON = 0.0001;

        params::Parameters params;

        params.setInt("FOO", 1);
        params.setFloat("BAR", 3.14);

        REQUIRE(params.getInt("FOO", 0) == 1);
        REQUIRE(std::abs(params.getFloat("BAR", 0.0) - 3.14) <= EPSILON);
        REQUIRE(std::abs(params.getFloat("BAT", 1.2345) - 1.2345) <= EPSILON);
    }
}
