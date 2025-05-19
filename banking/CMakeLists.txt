cmake_minimum_required(VERSION 3.14)
project(banking LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)


add_library(banking
    Account.cpp
    Account.h
    Transaction.cpp
    Transaction.h
)

target_include_directories(banking PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})


option(BUILD_TESTS "Build tests" ON)

if(BUILD_TESTS)
    enable_testing()
    

    add_subdirectory(third-party/gtest)


    file(GLOB TEST_SOURCES tests/*.cpp)

    add_executable(check ${TEST_SOURCES})


    target_link_libraries(check
        banking
        gtest_main
        gmock_main
    )


    add_test(NAME check COMMAND check)
endif()


option(ENABLE_COVERAGE "Enable coverage reporting" OFF)
if(ENABLE_COVERAGE)
    add_compile_options(--coverage -fprofile-arcs -ftest-coverage)
    add_link_options(--coverage)
endif()
