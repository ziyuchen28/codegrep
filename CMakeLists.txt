cmake_minimum_required(VERSION 3.20)
project(codegrep LANGUAGES CXX)

option(CODEGREP_BUILD_TESTS "Build codegrep tests" ON)
option(CODEGREP_BUILD_CLI "Build codegrep CLI/debug executable" ON)
set(CODEGREP_TEST_FIXTURE_ROOT "" CACHE PATH "Path to mini-java-playground fixture")
set(CODEGREP_DEFAULT_RG_PATH "" CACHE FILEPATH "Default rg path compiled into the library")

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_library(codegrep
    src/process_posix.cpp
    src/rg_command.cpp
    src/ripgrep.cpp
    src/scoring.cpp
    src/hit_scanner.cpp
    src/snippet_builder.cpp
    src/searcher.cpp
)

target_compile_features(codegrep PUBLIC cxx_std_20)

target_include_directories(codegrep
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src
)

if (CODEGREP_DEFAULT_RG_PATH)
    target_compile_definitions(codegrep PUBLIC CODEGREP_DEFAULT_RG_PATH="${CODEGREP_DEFAULT_RG_PATH}")
endif()

if (CODEGREP_BUILD_CLI)
    add_executable(codegrep_cli tools/codegrep_cli.cpp)
    target_link_libraries(codegrep_cli PRIVATE codegrep)
endif()

if (CODEGREP_BUILD_TESTS)
    enable_testing()

    add_executable(test_searcher tests/test_searcher.cpp)
    target_link_libraries(test_searcher PRIVATE codegrep)

    if (CODEGREP_TEST_FIXTURE_ROOT)
        target_compile_definitions(test_searcher PRIVATE CODEGREP_TEST_FIXTURE_ROOT="${CODEGREP_TEST_FIXTURE_ROOT}")
    endif()

    add_test(NAME test_searcher COMMAND test_searcher)
endif()

















