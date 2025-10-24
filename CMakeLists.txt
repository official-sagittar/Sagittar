cmake_minimum_required(VERSION 3.25)

project(Sagittar LANGUAGES CXX)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "Release" CACHE STRING "Build type")
endif()

if(DEFINED EXE AND NOT EXE STREQUAL "")
    set(EXE_NAME "${EXE}")
elseif(DEFINED ENV{EXE} AND NOT "$ENV{EXE}" STREQUAL "")
    set(EXE_NAME "$ENV{EXE}")
else()
    set(EXE_NAME "${PROJECT_NAME}")
endif()

if(DEFINED BIN_DIR AND NOT "${BIN_DIR}" STREQUAL "")
    set(BIN_DIR "${BIN_DIR}" CACHE PATH "Binary output directory")
elseif(DEFINED ENV{BIN_DIR} AND NOT "$ENV{BIN_DIR}" STREQUAL "")
    set(BIN_DIR "$ENV{BIN_DIR}" CACHE PATH "Binary output directory")
else()
    set(BIN_DIR "${CMAKE_SOURCE_DIR}/bin/${CMAKE_BUILD_TYPE}")
endif()

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_CXX_FLAGS "-Wall -Wextra -Wpedantic -march=native")

option(ENABLE_SAN "Enable Address and Undefined sanitizers" OFF)

file(GLOB_RECURSE CORE_SRC CONFIGURE_DEPENDS
    src/*.cpp
    src/*.h
)

list(FILTER CORE_SRC EXCLUDE REGEX "src/pch.cpp")

file(GLOB_RECURSE TEST_SRC CONFIGURE_DEPENDS
    test/*.cpp
    test/*.h
)

add_executable(${EXE_NAME})
target_precompile_headers(${EXE_NAME} PRIVATE src/pch.h)
target_include_directories(${EXE_NAME} PRIVATE src)

set_target_properties(${EXE_NAME} PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${BIN_DIR}
)

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

elseif(CMAKE_BUILD_TYPE STREQUAL "Release")
    target_sources(${EXE_NAME} PRIVATE ${CORE_SRC})
    target_compile_definitions(${EXE_NAME} PRIVATE NDEBUG)
    target_compile_options(${EXE_NAME} PRIVATE -O3)
endif()

if(ENABLE_SAN)
    message(STATUS "Enabling sanitizers: address + undefined")
    target_compile_options(${EXE_NAME} PRIVATE -fsanitize=address -fsanitize=undefined)
    target_link_options(${EXE_NAME} PRIVATE -fsanitize=address -fsanitize=undefined)
    target_compile_definitions(${EXE_NAME} PRIVATE ENABLE_SANITIZERS)
endif()

if(CMAKE_BUILD_TYPE MATCHES "^(Release|Test)$")
    include(CheckIPOSupported)
    check_ipo_supported(RESULT ipo_supported OUTPUT ipo_error)
    if(ipo_supported)
        set_property(TARGET ${EXE_NAME} PROPERTY INTERPROCEDURAL_OPTIMIZATION TRUE)
        message(STATUS "LTO enabled")
    else()
        message(WARNING "IPO/LTO not supported: ${ipo_error}")
    endif()
endif()

message(STATUS "")
message(STATUS "================ Sagittar Build Summary ================")
message(STATUS "Target:       ${EXE_NAME}")
message(STATUS "Output Dir:   ${BIN_DIR}")
message(STATUS "Compiler:     ${CMAKE_CXX_COMPILER}")
message(STATUS "Configuration:${CMAKE_BUILD_TYPE}")
message(STATUS "Sanitizers:   ${ENABLE_SAN}")
message(STATUS "========================================================")
message(STATUS "")
