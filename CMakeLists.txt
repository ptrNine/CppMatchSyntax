cmake_minimum_required(VERSION 3.14)
project(CppMatchSyntax)

set(CMAKE_CXX_STANDARD 17)

add_executable(examples examples.cpp)

if (ENABLE_TESTS)
    add_executable(Tests tests.cpp)
endif ()