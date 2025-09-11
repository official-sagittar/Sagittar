cmake_minimum_required(VERSION 3.16)
project(Sagittar LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Configurations
set(CMAKE_CONFIGURATION_TYPES "Debug;Test;Sanity;Release" CACHE STRING "" FORCE)

# Binary name from OpenBench (EXE=...)
if (DEFINED EXE)
    set(EXE_NAME ${EXE})
else()
    set(EXE_NAME Sagittar)
endif()

# Place binaries next to Makefile
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR})

# ===========================
# Sources
# ===========================
file(GLOB_RECURSE SAGITTAR_HEADERS src/*.h)
file(GLOB_RECURSE SAGITTAR_SOURCES src/*.cpp)

# ===========================
# Main engine target
# ===========================
add_executable(${EXE_NAME} ${SAGITTAR_HEADERS} ${SAGITTAR_SOURCES})
target_include_directories(${EXE_NAME} PRIVATE src)

# Precompiled headers
if (CMAKE_VERSION VERSION_GREATER_EQUAL 3.16)
    target_precompile_headers(${EXE_NAME} PRIVATE src/pch.h)
endif()

# Common warnings + arch
target_compile_options(${EXE_NAME} PRIVATE -Wall -Wextra -Wpedantic -march=native)

# Linker flags (-static only where allowed)
if (APPLE)
    # no -static on macOS
else()
    target_link_options(${EXE_NAME} PRIVATE -static)
endif()

# ===========================
# Sanitizer option
# ===========================
option(ENABLE_SAN "Enable ASan and UBSan" OFF)
if (ENABLE_SAN)
    target_compile_options(${EXE_NAME} PRIVATE -fsanitize=undefined -fsanitize=address)
    target_link_options(${EXE_NAME} PRIVATE -fsanitize=undefined -fsanitize=address)
endif()

# ===========================
# PGO options
# ===========================
option(PGO_GENERATE "Enable Profile-Guided Optimization (generate profile)" OFF)
option(PGO_USE "Enable Profile-Guided Optimization (use profile)" OFF)
set(PGO_PROFILE_DIR "${CMAKE_SOURCE_DIR}/pgo-data" CACHE STRING "Directory for PGO profile data")

if (PGO_GENERATE)
    message(STATUS "Building with PGO (generate) -> ${PGO_PROFILE_DIR}")
    file(MAKE_DIRECTORY ${PGO_PROFILE_DIR})
    target_compile_options(${EXE_NAME} PRIVATE -fprofile-generate=${PGO_PROFILE_DIR})
    target_link_options(${EXE_NAME} PRIVATE -fprofile-generate=${PGO_PROFILE_DIR})
elseif (PGO_USE)
    message(STATUS "Building with PGO (use) -> ${PGO_PROFILE_DIR}")
    target_compile_options(${EXE_NAME} PRIVATE -fprofile-use=${PGO_PROFILE_DIR} -fprofile-correction)
    target_link_options(${EXE_NAME} PRIVATE -fprofile-use=${PGO_PROFILE_DIR} -fprofile-correction)
endif()

# ===========================
# Config-specific settings
# ===========================

# Debug: add DEBUG macro (CMake already adds -g)
set(CMAKE_CXX_FLAGS_DEBUG "-g")
target_compile_definitions(${EXE_NAME} PRIVATE $<$<CONFIG:Debug>:DEBUG>)

# ---------------------------------------
# Exclude main.cpp for tests/sanity builds
# ---------------------------------------
set(SAGITTAR_SOURCES_NO_MAIN ${SAGITTAR_SOURCES})
list(FILTER SAGITTAR_SOURCES_NO_MAIN EXCLUDE REGEX "src/main.cpp")

# Test (Debug-like)
add_executable(${EXE_NAME}_test
    ${SAGITTAR_HEADERS}
    ${SAGITTAR_SOURCES_NO_MAIN}
    test/test_main.cpp
    test/test_movegen.cpp
    test/test_movepicker.cpp
    test/test_perft.cpp
    test/test_position.cpp
)
target_include_directories(${EXE_NAME}_test PRIVATE
    src
    test/lib/doctest
)
target_compile_definitions(${EXE_NAME}_test PRIVATE DEBUG TEST)
target_compile_options(${EXE_NAME}_test PRIVATE -Wall -Wextra -Wpedantic -march=native)
if (APPLE)
    # no -static on macOS
else()
    target_link_options(${EXE_NAME}_test PRIVATE -static)
endif()

# Sanity (optimized test build)
add_executable(${EXE_NAME}_sanity
    ${SAGITTAR_HEADERS}
    ${SAGITTAR_SOURCES_NO_MAIN}
    test/test_main.cpp
    test/test_movegen.cpp
    test/test_movepicker.cpp
    test/test_perft.cpp
    test/test_position.cpp
)
target_include_directories(${EXE_NAME}_sanity PRIVATE
    src
    test/lib/doctest
)
target_compile_definitions(${EXE_NAME}_sanity PRIVATE TEST)
# Sanity is not a built-in CMake config, so give it -O3 explicitly
target_compile_options(${EXE_NAME}_sanity PRIVATE -O3 -march=native)
if (APPLE)
    # no -static on macOS
else()
    target_link_options(${EXE_NAME}_sanity PRIVATE -static)
endif()

# ===== Release =====
# Remove manual -O3 (CMake's Release already adds -O3 -DNDEBUG)
# Also DON'T add -flto here; use IPO/LTO below so compile steps get it too.

# ===========================
# Enable IPO/LTO cleanly
# ===========================
include(CheckIPOSupported)
check_ipo_supported(RESULT IPO_SUPPORTED OUTPUT IPO_ERROR)
if (IPO_SUPPORTED)
    message(STATUS "IPO/LTO: enabled for Release and Sanity")
    # LTO on for Release builds of the main engine
    set_property(TARGET ${EXE_NAME} PROPERTY
        INTERPROCEDURAL_OPTIMIZATION $<$<CONFIG:Release>:TRUE>)
    # LTO on for the Sanity target (always)
    set_property(TARGET ${EXE_NAME}_sanity PROPERTY INTERPROCEDURAL_OPTIMIZATION TRUE)
else()
    message(STATUS "IPO/LTO not supported: ${IPO_ERROR}")
endif()

# ===========================
# Tests
# ===========================
enable_testing()
add_test(NAME Sagittar_UnitTests   COMMAND ${EXE_NAME}_test)
add_test(NAME Sagittar_SanityTests COMMAND ${EXE_NAME}_sanity)

# ===========================
# Build configuration summary
# ===========================
message(STATUS "----------------------------------------")
message(STATUS " Sagittar Build Configuration Summary")
message(STATUS "----------------------------------------")
message(STATUS "C++ Compiler : ${CMAKE_CXX_COMPILER}")
message(STATUS "C++ Standard : ${CMAKE_CXX_STANDARD}")
message(STATUS "Build Type   : ${CMAKE_BUILD_TYPE}")
message(STATUS "Output Dir   : ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}")
message(STATUS "")
message(STATUS "Preprocessor macros:")
message(STATUS "  Debug    : DEBUG")
message(STATUS "  Test     : DEBUG, TEST")
message(STATUS "  Sanity   : TEST")
message(STATUS "  Release  : NDEBUG (added by CMake automatically)")
message(STATUS "----------------------------------------")
