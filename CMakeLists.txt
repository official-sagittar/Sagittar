cmake_minimum_required(VERSION 3.25)

include(FetchContent)
include(CheckIPOSupported)

# ----------------------------------------------------------
# Supported build configurations
# ----------------------------------------------------------
set(SUPPORTED_CONFIGS Debug Release Test Tune)

# Initialize configurations before project()
# for multi-config generators.
get_property(IS_MULTI_CONFIG GLOBAL PROPERTY GENERATOR_IS_MULTI_CONFIG)

if(IS_MULTI_CONFIG)
    if(NOT DEFINED CMAKE_CONFIGURATION_TYPES)
        set(CMAKE_CONFIGURATION_TYPES
            "${SUPPORTED_CONFIGS}"
            CACHE STRING "Available build configurations"
        )
    endif()
endif()

project(Sagittar LANGUAGES CXX)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

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
if(CMAKE_CONFIGURATION_TYPES)
    # Multi-config generator
    set(AVAILABLE_CONFIGS ${CMAKE_CONFIGURATION_TYPES})

    foreach(config IN LISTS AVAILABLE_CONFIGS)
        if(NOT config IN_LIST SUPPORTED_CONFIGS)
            message(FATAL_ERROR
                "Unsupported configuration: ${config}. "
                "Supported configurations: ${SUPPORTED_CONFIGS}"
            )
        endif()
    endforeach()
else()
    # Single-config generator
    if(NOT CMAKE_BUILD_TYPE)
        set(CMAKE_BUILD_TYPE Release CACHE STRING
            "Build type" FORCE
        )
    endif()

    if(NOT CMAKE_BUILD_TYPE IN_LIST SUPPORTED_CONFIGS)
        message(FATAL_ERROR
            "Unsupported build type: ${CMAKE_BUILD_TYPE}. "
            "Supported configurations: ${SUPPORTED_CONFIGS}"
        )
    endif()

    set_property(CACHE CMAKE_BUILD_TYPE PROPERTY
        STRINGS ${SUPPORTED_CONFIGS}
    )

    set(AVAILABLE_CONFIGS ${CMAKE_BUILD_TYPE})
endif()

# ----------------------------------------------------------
# Executable name
# ----------------------------------------------------------
if(DEFINED EXE AND NOT EXE STREQUAL "")
    set(EXE_NAME "${EXE}")
elseif(DEFINED ENV{EXE} AND NOT "$ENV{EXE}" STREQUAL "")
    set(EXE_NAME "$ENV{EXE}")
else()
    set(EXE_NAME "${PROJECT_NAME}")
endif()

# ----------------------------------------------------------
# Output directory
# ----------------------------------------------------------
if(DEFINED BIN_DIR AND NOT "${BIN_DIR}" STREQUAL "")
    set(BIN_DIR "${BIN_DIR}" CACHE PATH
        "Binary output directory"
    )
elseif(DEFINED ENV{BIN_DIR} AND NOT "$ENV{BIN_DIR}" STREQUAL "")
    set(BIN_DIR "$ENV{BIN_DIR}" CACHE PATH
        "Binary output directory"
    )
else()
    set(BIN_DIR "${CMAKE_SOURCE_DIR}/bin/$<CONFIG>")
endif()

# ----------------------------------------------------------
# Target setup
# ----------------------------------------------------------
add_executable(${EXE_NAME})

set_target_properties(${EXE_NAME} PROPERTIES
    CXX_STANDARD 20
    CXX_STANDARD_REQUIRED ON
    CXX_EXTENSIONS OFF
    RUNTIME_OUTPUT_DIRECTORY "${BIN_DIR}"
)

target_compile_options(${EXE_NAME} PRIVATE
    -Wall
    -Wextra
    -Wpedantic
    -march=native

    $<$<CONFIG:Debug>:-g;-O0;-fsanitize=address,undefined>
    $<$<CONFIG:Release,Test,Tune>:-O3;-Werror>
)

target_link_options(${EXE_NAME} PRIVATE
    $<$<CONFIG:Debug>:-fsanitize=address,undefined>
)

target_precompile_headers(${EXE_NAME} PRIVATE
    "${CMAKE_CURRENT_SOURCE_DIR}/src/commons/pch.h"
)

target_include_directories(${EXE_NAME} PRIVATE
    "${CMAKE_CURRENT_SOURCE_DIR}/src"
)

target_compile_definitions(${EXE_NAME} PRIVATE
    $<$<CONFIG:Debug,Test>:DEBUG>
    $<$<CONFIG:Test>:TEST>
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
target_sources(${EXE_NAME} PRIVATE
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
if("Test" IN_LIST AVAILABLE_CONFIGS)
    set(DOCTEST_NO_INSTALL ON CACHE BOOL "" FORCE)
    set(DOCTEST_WITH_TESTS OFF CACHE BOOL "" FORCE)
    set(DOCTEST_WITH_MAIN_IN_STATIC_LIB OFF CACHE BOOL "" FORCE)

    FetchContent_Declare(doctest
        GIT_REPOSITORY https://github.com/doctest/doctest.git
        GIT_TAG        v2.5.2
    )

    FetchContent_MakeAvailable(doctest)

    target_link_libraries(${EXE_NAME} PRIVATE
        "$<$<CONFIG:Test>:doctest::doctest>"
    )
endif()

# ----------------------------------------------------------
# dp::thread-pool (v0.7.0)
# Repo: https://github.com/ptsouchlos/thread-pool
# ----------------------------------------------------------
if("Tune" IN_LIST AVAILABLE_CONFIGS)
    set(TP_BUILD_TESTS OFF CACHE BOOL "" FORCE)
    set(TP_BUILD_BENCHMARKS OFF CACHE BOOL "" FORCE)
    set(TP_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)

    FetchContent_Declare(dp_thread_pool
        GIT_REPOSITORY https://github.com/ptsouchlos/thread-pool.git
        GIT_TAG        0.7.0
    )

    FetchContent_MakeAvailable(dp_thread_pool)

    target_link_libraries(${EXE_NAME} PRIVATE
        "$<$<CONFIG:Tune>:dp::thread-pool>"
    )
endif()

# ----------------------------------------------------------
# Testing (CTest)
# ----------------------------------------------------------
if("Test" IN_LIST AVAILABLE_CONFIGS)
    enable_testing()

    if(IS_MULTI_CONFIG)
        add_test(
            NAME SagittarTests
            COMMAND $<TARGET_FILE:${EXE_NAME}>
            CONFIGURATIONS Test
        )
    else()
        add_test(
            NAME SagittarTests
            COMMAND $<TARGET_FILE:${EXE_NAME}>
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
  set_target_properties(${EXE_NAME} PROPERTIES
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
    target_link_options(${EXE_NAME} PRIVATE
        $<$<CONFIG:Release>:-static>
    )
endif()

# ----------------------------------------------------------
# Linker selection
# ----------------------------------------------------------
if(CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
    target_link_options(${EXE_NAME} PRIVATE
        -fuse-ld=lld
    )
endif()
