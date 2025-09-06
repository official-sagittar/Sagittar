#define DOCTEST_CONFIG_IMPLEMENT
#include "core/movegen.h"
#include "core/position.h"
#include "doctest/doctest.h"
#include "eval/hce/eval.h"

int main(int argc, char** argv) {

    sagittar::core::position_init();
    sagittar::core::movegen_init();
    sagittar::eval::hce::eval_init();

    doctest::Context context;
    context.applyCommandLine(argc, argv);

    int res = context.run();   // run
    if (context.shouldExit())  // important - query flags (and --exit) rely on the user doing this
        return res;            // propagate the result of the tests

    return res;
}
