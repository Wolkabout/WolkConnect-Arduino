cmake_minimum_required(VERSION 3.10.2)
project(testWolkConnector-Arduino)

set(LIB "/home/mvukovic/googletest/googlemock/lib/")

file(GLOB SOURCES "../src/utility/*.cpp")

include_directories(../src/ ../src/utility/)
add_executable(${PROJECT_NAME} OutboundMessageFactoryTest.cpp ${SOURCES})

# Locate GTest
find_package(GTest REQUIRED)

include_directories(${GTEST_INCLUDE_DIRS})
target_link_libraries(${PROJECT_NAME} gtest gtest_main pthread)
