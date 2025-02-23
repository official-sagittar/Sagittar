#include "doctest/doctest.h"
#include "params.h"
#include "pch.h"

using namespace sagittar;

TEST_SUITE("Params") {

    TEST_CASE("Params::set and Params::get") {
        const float EPSILON = 0.0001;

        parameters::ParameterStore params;

        params.set<int>("FOO", 1);
        params.set<float>("BAR", 3.14);

        REQUIRE(params.get<int>("FOO", 0) == 1);
        REQUIRE(std::abs(params.get<float>("BAR", 0.0) - 3.14) <= EPSILON);
        REQUIRE(std::abs(params.get<float>("BAT", 1.2345) - 1.2345) <= EPSILON);
    }
}
