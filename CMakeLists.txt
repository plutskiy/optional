cmake_minimum_required(VERSION 3.14)
project(optional)

include(FetchContent)
FetchContent_Declare(
        googletest
        GIT_REPOSITORY https://github.com/google/googletest.git
        GIT_TAG        release-1.12.1
)
FetchContent_MakeAvailable(googletest)

set(CMAKE_CXX_STANDARD 23)
set(TEST_NAME ${PROJECT_NAME}_tests)
add_executable(${TEST_NAME} optional_test.cpp bmstu_optional.h)
target_link_libraries(${TEST_NAME} gtest_main)

enable_testing()
include(GoogleTest)
gtest_discover_tests(${TEST_NAME})