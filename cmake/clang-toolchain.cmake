cmake_minimum_required(VERSION 3.25)

# 1. If user explicitly provided compilers, respect them
if(DEFINED CMAKE_CXX_COMPILER)
    message(STATUS "Using user-specified compilers:")
    message(STATUS "  C++: ${CMAKE_CXX_COMPILER}")
else()
    # 2. Try system Clang first
    find_program(CLANGXX_PATH clang++)
    if(CLANGXX_PATH)
        message(STATUS "Using system Clang found in PATH:")
        message(STATUS "  ${CLANGXX_PATH}")
        set(CMAKE_CXX_COMPILER ${CLANGXX_PATH} CACHE STRING "C++ compiler")
    else()
        # 3. Fallback to GCC
        find_program(GXX_PATH g++)
        if(GXX_PATH)
            message(WARNING "Clang not found; using GCC from PATH:")
            message(STATUS "  ${GXX_PATH}")
            set(CMAKE_CXX_COMPILER ${GXX_PATH} CACHE STRING "C++ compiler")
        else()
            message(FATAL_ERROR "No suitable compiler found (clang++ or g++ required).")
        endif()
    endif()
endif()

# 4. Common flags
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_CXX_FLAGS_INIT "-Wall -Wextra -Wpedantic -march=native")
set(CMAKE_EXE_LINKER_FLAGS_INIT "-fuse-ld=lld")

# 5. Compiler-specific tweaks
if(CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    set(CMAKE_CXX_FLAGS_INIT "${CMAKE_CXX_FLAGS_INIT} -stdlib=libc++")
elseif(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    find_program(GOLD_LD ld.gold)
    if(GOLD_LD)
        set(CMAKE_EXE_LINKER_FLAGS_INIT "-fuse-ld=gold")
    endif()
endif()

# 6. Default build type
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "Release" CACHE STRING "Build type")
endif()

# 7. Disallow MSVC
if(CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
    message(FATAL_ERROR "MSVC is not supported. Please use LLVM Clang or GCC.")
endif()

# 8. Final summary
message(STATUS "Configuration:")
message(STATUS "  C++ compiler: ${CMAKE_CXX_COMPILER}")
message(STATUS "  Build type:   ${CMAKE_BUILD_TYPE}")
