cmake_minimum_required(VERSION 3.25)

project(
    SerialPacketParserModuleTests
    LANGUAGES CXX
)

set(EXPORT_COMPILE_COMMANDS ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(FetchContent)

FetchContent_Declare(
    googletest
    GIT_REPOSITORY https://github.com/google/googletest.git
    GIT_TAG v1.14.0
)
FetchContent_MakeAvailable(googletest)

add_executable(SerialPacketParserTests
                tests.cpp
                ../parser.c
                ../ring_buffer.c)

target_include_directories(SerialPacketParserTests
                            PRIVATE ${CMAKE_SOURCE_DIR})

target_link_libraries(SerialPacketParserTests
                        PRIVATE gtest_main)