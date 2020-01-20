cmake_minimum_required(VERSION 3.10)
project(CppMatchSyntax)

set(CMAKE_CXX_STANDARD 17)

if (BUILD_EXAMPLES)
    add_executable(examples examples.cpp)
endif()

if (BUILD_TESTS)
    add_executable(Tests tests.cpp)
endif()