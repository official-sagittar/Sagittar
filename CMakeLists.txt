cmake_minimum_required(VERSION 3.25)
project(Sagittar LANGUAGES CXX)

# ----------------------------------------------------------
# Disallow MSVC
# ----------------------------------------------------------
if(MSVC)
    message(FATAL_ERROR "Sagittar does not support MSVC. Please use Clang (MSYS2/clang64) or GCC.")
endif()

# ----------------------------------------------------------
# Default build type
# ----------------------------------------------------------
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "Release" CACHE STRING "Build type")
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
    set(BIN_DIR "${BIN_DIR}" CACHE PATH "Binary output directory")
elseif(DEFINED ENV{BIN_DIR} AND NOT "$ENV{BIN_DIR}" STREQUAL "")
    set(BIN_DIR "$ENV{BIN_DIR}" CACHE PATH "Binary output directory")
else()
    set(BIN_DIR "${CMAKE_SOURCE_DIR}/bin/${CMAKE_BUILD_TYPE}")
endif()

# ----------------------------------------------------------
# Compiler setup
# ----------------------------------------------------------
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
option(ENABLE_SAN "Enable Address and Undefined sanitizers" OFF)

# Only affect this target (avoid global flag pollution)
add_executable(${EXE_NAME})
target_compile_options(${EXE_NAME} PRIVATE -Wall -Wextra -Wpedantic -march=native)
target_precompile_headers(${EXE_NAME} PRIVATE src/commons/pch.h)
target_include_directories(${EXE_NAME} PRIVATE src)

set_target_properties(${EXE_NAME} PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${BIN_DIR}
    POSITION_INDEPENDENT_CODE OFF
)

# ----------------------------------------------------------
# Source files
# ----------------------------------------------------------
file(GLOB_RECURSE CORE_SRC CONFIGURE_DEPENDS src/**.cpp src/**.h)
list(FILTER CORE_SRC EXCLUDE REGEX "src/commons/pch.cpp")
list(FILTER CORE_SRC EXCLUDE REGEX "src/eval/hce/tuner/*")

file(GLOB_RECURSE TEST_SRC CONFIGURE_DEPENDS test/*.cpp test/*.h)
file(GLOB_RECURSE TUNE_SRC CONFIGURE_DEPENDS src/eval/hce/tuner/*.cpp src/eval/hce/tuner/*.h)

# ----------------------------------------------------------
# Build type configurations
# ----------------------------------------------------------
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    target_sources(${EXE_NAME} PRIVATE ${CORE_SRC})
    target_compile_definitions(${EXE_NAME} PRIVATE DEBUG)
    target_compile_options(${EXE_NAME} PRIVATE -O0 -g)

elseif(CMAKE_BUILD_TYPE STREQUAL "Test")
    set(CORE_SRC_FOR_TEST ${CORE_SRC})
    list(FILTER CORE_SRC_FOR_TEST EXCLUDE REGEX "src/main.cpp")
    target_sources(${EXE_NAME} PRIVATE ${CORE_SRC_FOR_TEST} ${TEST_SRC})
    target_include_directories(${EXE_NAME} PRIVATE test/lib/doctest)
    target_compile_definitions(${EXE_NAME} PRIVATE DEBUG TEST)
    target_compile_options(${EXE_NAME} PRIVATE -O3)

elseif(CMAKE_BUILD_TYPE STREQUAL "Tune")
    target_sources(${EXE_NAME} PRIVATE ${CORE_SRC} ${TUNE_SRC})
    target_compile_definitions(${EXE_NAME} PRIVATE NDEBUG EXTERNAL_TUNE)
    target_compile_options(${EXE_NAME} PRIVATE -O3)

elseif(CMAKE_BUILD_TYPE STREQUAL "Release")
    target_sources(${EXE_NAME} PRIVATE ${CORE_SRC})
    target_compile_definitions(${EXE_NAME} PRIVATE NDEBUG)
    target_compile_options(${EXE_NAME} PRIVATE -O3)
endif()

# ----------------------------------------------------------
# Sanitizers
# ----------------------------------------------------------
if(ENABLE_SAN)
    message(STATUS "Enabling sanitizers: address + undefined")
    target_compile_options(${EXE_NAME} PRIVATE -fsanitize=address -fsanitize=undefined)
    target_link_options(${EXE_NAME} PRIVATE -fsanitize=address -fsanitize=undefined)
    target_compile_definitions(${EXE_NAME} PRIVATE ENABLE_SANITIZERS)
endif()

# ----------------------------------------------------------
# Interprocedural Optimization (LTO)
# ----------------------------------------------------------
set(LTO_STATUS "OFF")

if(CMAKE_BUILD_TYPE MATCHES "^(Release|Tune|Test)$")
    include(CheckIPOSupported)
    check_ipo_supported(RESULT ipo_supported OUTPUT ipo_error)
    if(ipo_supported)
        set_property(TARGET ${EXE_NAME} PROPERTY INTERPROCEDURAL_OPTIMIZATION TRUE)
        set(LTO_STATUS "ON (IPO enabled)")
        message(STATUS "LTO enabled")
    else()
        set(LTO_STATUS "OFF (IPO not supported: ${ipo_error})")
        message(WARNING "IPO/LTO not supported: ${ipo_error}")
    endif()
endif()

# ----------------------------------------------------------
# Static linking rules (Windows + Linux)
# ----------------------------------------------------------
set(STATIC_STATUS "OFF")

if(NOT ENABLE_SAN)
    if(APPLE)
        message(STATUS "Static linking skipped on macOS (unsupported)")
    else()
        target_link_options(${EXE_NAME} PRIVATE -static)
        set(STATIC_STATUS "ON")
    endif()
endif()

if(ENABLE_SAN AND STATIC_STATUS STREQUAL "ON")
    message(WARNING "Sanitizers and full static linking are incompatible; expect linker errors.")
endif()

# ----------------------------------------------------------
# Linker selection
# ----------------------------------------------------------
if(APPLE)
    set(LINKER_STATUS "Apple ld64 (default)")

elseif(CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    add_link_options(-fuse-ld=lld)
    set(LINKER_STATUS "LLD (forced for Clang)")

elseif(CMAKE_CXX_COMPILER_ID MATCHES "GNU")
    set(LINKER_STATUS "System ld (default for GCC)")

else()
    set(LINKER_STATUS "Unknown linker (default)")
endif()

get_target_property(SG_COMPILE_FLAGS ${EXE_NAME} COMPILE_OPTIONS)
get_target_property(SG_LINK_FLAGS ${EXE_NAME} LINK_OPTIONS)

if (NOT SG_COMPILE_FLAGS OR SG_COMPILE_FLAGS STREQUAL "SG_COMPILE_FLAGS-NOTFOUND")
    set(SG_COMPILE_FLAGS "(none)")
endif()

if (NOT SG_LINK_FLAGS OR SG_LINK_FLAGS STREQUAL "SG_LINK_FLAGS-NOTFOUND")
    set(SG_LINK_FLAGS "(none)")
endif()

# ----------------------------------------------------------
# Build summary
# ----------------------------------------------------------
message(STATUS "")
message(STATUS "================ Sagittar Build Summary ================")
message(STATUS "Target:                 ${EXE_NAME}")
message(STATUS "Output Dir:             ${BIN_DIR}")
message(STATUS "Compiler Path:          ${CMAKE_CXX_COMPILER}")
message(STATUS "Compiler:               ${CMAKE_CXX_COMPILER_ID} ${CMAKE_CXX_COMPILER_VERSION}")
message(STATUS "Linker:                 ${LINKER_STATUS}")
message(STATUS "Configuration:          ${CMAKE_BUILD_TYPE}")
message(STATUS "Sanitizers:             ${ENABLE_SAN}")
message(STATUS "Static Link:            ${STATIC_STATUS}")
message(STATUS "Target compile flags:   ${SG_COMPILE_FLAGS}")
message(STATUS "Target link flags:      ${SG_LINK_FLAGS}")
message(STATUS "Link Time Optimization: ${LTO_STATUS}")
message(STATUS "========================================================")
message(STATUS "")
