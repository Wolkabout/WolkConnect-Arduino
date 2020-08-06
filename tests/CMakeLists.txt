cmake_minimum_required(VERSION 3.10.2)
project(testWolkConnector-Arduino)

file(GLOB SOURCES "../src/utility/*.cpp")
file(GLOB TESTS "../tests/*.cpp")

include_directories(../src/ ../src/utility/)

add_executable(${PROJECT_NAME} ${TESTS} ${SOURCES})

# Locate GTest
find_package(GTest REQUIRED)

include_directories(${GTEST_INCLUDE_DIRS})
target_link_libraries(${PROJECT_NAME} gtest gtest_main pthread)
