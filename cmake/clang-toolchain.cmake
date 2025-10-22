cmake_minimum_required(VERSION 3.25)
message(STATUS "=== Sagittar toolchain ===")

# 1. Disallow MSVC
if(CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
    message(FATAL_ERROR "MSVC is not supported. Please use LLVM Clang.")
endif()

# 2. Prefer user-provided compiler if set
if(DEFINED CMAKE_CXX_COMPILER)
    message(STATUS "Using user-specified compiler: ${CMAKE_CXX_COMPILER}")
else()
    find_program(CLANGXX_PATH clang++)
    if(CLANGXX_PATH)
        set(CMAKE_CXX_COMPILER "${CLANGXX_PATH}" CACHE STRING "C++ compiler" FORCE)
    else()
        message(FATAL_ERROR "clang++ not found in PATH. Please open from MSYS2 clang64 shell.")
    endif()
endif()

# 3. Common settings
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_CXX_FLAGS "-Wall -Wextra -Wpedantic -march=native")
set(CMAKE_EXE_LINKER_FLAGS "-fuse-ld=lld")

# 4. Detect platform and configure paths
if(WIN32)
    set(CLANG_PATH "C:/msys64/clang64")
    link_directories("${CLANG_PATH}/lib")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -stdlib=libc++ -static")
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -fuse-ld=lld -static -lc++ -lc++abi -lunwind -lwinpthread")
endif()

# 5. Default build type
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "Release" CACHE STRING "Build type" FORCE)
endif()

# 6. Summary
message(STATUS "")
message(STATUS "=== Toolchain Summary ===")
message(STATUS "C++ Compiler : ${CMAKE_CXX_COMPILER}")
message(STATUS "C++ Flags    : ${CMAKE_CXX_FLAGS}")
message(STATUS "Linker Flags : ${CMAKE_EXE_LINKER_FLAGS}")
message(STATUS "Build Type   : ${CMAKE_BUILD_TYPE}")
message(STATUS "=================================")
message(STATUS "")
