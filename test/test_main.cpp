#define DOCTEST_CONFIG_IMPLEMENT
#include "doctest/doctest.h"
#include "sagittar/core/board.h"

int main(int argc, char** argv) {

    sagittar::core::board::Board::initialize();

    doctest::Context context;
    context.applyCommandLine(argc, argv);

    int res = context.run();   // run
    if (context.shouldExit())  // important - query flags (and --exit) rely on the user doing this
        return res;            // propagate the result of the tests

    return res;
}
