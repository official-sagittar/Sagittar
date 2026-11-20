cmake_minimum_required(VERSION 3.25)

include(FetchContent)
include(CheckIPOSupported)

project(Sagittar LANGUAGES CXX)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(SUPPORTED_CONFIGS Debug Release Test Tune)
set(DOCTEST_ARGS "" CACHE STRING "Arguments passed to doctest")

# ----------------------------------------------------------
# Disallow MSVC
# ----------------------------------------------------------
if(MSVC)
    message(FATAL_ERROR
        "Sagittar does not support MSVC. Please use Clang or GCC."
    )
endif()

# ----------------------------------------------------------
# Build configuration validation
# ----------------------------------------------------------
if(NOT CMAKE_BUILD_TYPE IN_LIST SUPPORTED_CONFIGS)
    message(FATAL_ERROR
        "Unsupported build type: ${CMAKE_BUILD_TYPE}. "
        "Supported configurations: ${SUPPORTED_CONFIGS}"
    )
endif()

# ----------------------------------------------------------
# Target setup
# ----------------------------------------------------------
add_executable(${PROJECT_NAME})

set_target_properties(${PROJECT_NAME} PROPERTIES
    CXX_STANDARD 20
    CXX_STANDARD_REQUIRED ON
    CXX_EXTENSIONS OFF
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/bin/$<CONFIG>"
)

target_compile_options(${PROJECT_NAME} PRIVATE
    -Wall
    -Wextra
    -Wpedantic
    -march=native

    $<$<CONFIG:Debug,Test>:-g;-O0;-fsanitize=address,undefined>
    $<$<CONFIG:Release,Tune>:-O3;-Werror>
)

target_link_options(${PROJECT_NAME} PRIVATE
    $<$<CONFIG:Debug,Test>:-fsanitize=address,undefined>
)

target_precompile_headers(${PROJECT_NAME} PRIVATE
    "${CMAKE_CURRENT_SOURCE_DIR}/src/commons/pch.h"
)

target_include_directories(${PROJECT_NAME} PRIVATE
    "${CMAKE_CURRENT_SOURCE_DIR}/src"
)

target_compile_definitions(${PROJECT_NAME} PRIVATE
    $<$<CONFIG:Debug,Test>:DEBUG>
    $<$<CONFIG:Tune>:EXTERNAL_TUNE>
    $<$<CONFIG:Release,Tune>:NDEBUG>
)

# ----------------------------------------------------------
# Source files
# ----------------------------------------------------------
file(GLOB_RECURSE CORE_SRC CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/*.h"
)

file(GLOB_RECURSE TEST_SRC CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/test/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/test/*.h"
)

file(GLOB_RECURSE TUNE_SRC CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/src/eval/hce/tuner/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/eval/hce/tuner/*.h"
)

# Exclude tuner sources from core sources
list(FILTER CORE_SRC EXCLUDE
    REGEX "/src/eval/hce/tuner/"
)

# Remove the normal entry point from common sources
set(MAIN_SRC "${CMAKE_CURRENT_SOURCE_DIR}/src/main.cpp")
list(REMOVE_ITEM CORE_SRC "${MAIN_SRC}")

# Common sources are compiled in every configuration.
target_sources(${PROJECT_NAME} PRIVATE
    ${CORE_SRC}

    # Normal executable entry point
    "$<$<NOT:$<CONFIG:Test>>:${MAIN_SRC}>"

    # Test configuration
    "$<$<CONFIG:Test>:${TEST_SRC}>"

    # Tune configuration
    "$<$<CONFIG:Tune>:${TUNE_SRC}>"
)

# ----------------------------------------------------------
# Libraries
# ----------------------------------------------------------

# ----------------------------------------------------------
# doctest (v2.5.2, header-only)
# Repo: https://github.com/doctest/doctest
# ----------------------------------------------------------
if(CMAKE_BUILD_TYPE STREQUAL "Test")
    set(DOCTEST_NO_INSTALL ON CACHE BOOL "" FORCE)
    set(DOCTEST_WITH_TESTS OFF CACHE BOOL "" FORCE)
    set(DOCTEST_WITH_MAIN_IN_STATIC_LIB OFF CACHE BOOL "" FORCE)

    FetchContent_Declare(doctest
        GIT_REPOSITORY https://github.com/doctest/doctest.git
        GIT_TAG        v2.5.2
    )

    FetchContent_MakeAvailable(doctest)

    target_link_libraries(${PROJECT_NAME} PRIVATE doctest::doctest)
endif()

# ----------------------------------------------------------
# dp::thread-pool (v0.7.0)
# Repo: https://github.com/ptsouchlos/thread-pool
# ----------------------------------------------------------
if(CMAKE_BUILD_TYPE STREQUAL "Tune")
    set(TP_BUILD_TESTS OFF CACHE BOOL "" FORCE)
    set(TP_BUILD_BENCHMARKS OFF CACHE BOOL "" FORCE)
    set(TP_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)

    FetchContent_Declare(dp_thread_pool
        GIT_REPOSITORY https://github.com/ptsouchlos/thread-pool.git
        GIT_TAG        0.7.0
    )

    FetchContent_MakeAvailable(dp_thread_pool)

    target_link_libraries(${PROJECT_NAME} PRIVATE dp::thread-pool)
endif()

# ----------------------------------------------------------
# Testing (CTest)
# ----------------------------------------------------------

if(CMAKE_BUILD_TYPE STREQUAL "Test")
    enable_testing()

    if(DOCTEST_ARGS)
        add_test(
            NAME SagittarTests
            COMMAND $<TARGET_FILE:${PROJECT_NAME}> "${DOCTEST_ARGS}"
        )
    else()
        add_test(
            NAME SagittarTests
            COMMAND $<TARGET_FILE:${PROJECT_NAME}>
        )
    endif()
endif()

# ----------------------------------------------------------
# Interprocedural Optimization (LTO)
# ----------------------------------------------------------
check_ipo_supported(
  RESULT ipo_supported
  OUTPUT ipo_error
)

if(ipo_supported)
  set_target_properties(${PROJECT_NAME} PROPERTIES
        INTERPROCEDURAL_OPTIMIZATION FALSE
        INTERPROCEDURAL_OPTIMIZATION_RELEASE TRUE
        INTERPROCEDURAL_OPTIMIZATION_TEST TRUE
        INTERPROCEDURAL_OPTIMIZATION_TUNE TRUE
  )
else()
    message(STATUS
        "IPO/LTO not supported: ${ipo_error}"
    )
endif()

# ----------------------------------------------------------
# Static linking
# ----------------------------------------------------------
if(NOT APPLE)
    target_link_options(${PROJECT_NAME} PRIVATE
        $<$<CONFIG:Release>:-static>
    )
endif()

# ----------------------------------------------------------
# Linker selection
# ----------------------------------------------------------
if(CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
    target_link_options(${PROJECT_NAME} PRIVATE
        -fuse-ld=lld
    )
endif()
