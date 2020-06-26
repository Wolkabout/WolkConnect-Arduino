cmake_minimum_required(VERSION 3.10.2)
project(testWolkConnector-Arduino)

set(LIB "/home/mvukovic/googletest/googlemock/lib/")

file(GLOB SOURCES "../src/utility/*.cpp")

#include(ExternalProject)

#ExternalProject_Add(libgtest
#    GIT_REPOSITORY "https://github.com/google/googletest"
#    GIT_TAG "release-${GTEST_VERSION}"
#    CMAKE_ARGS -DCMAKE_INSTALL_LIBDIR=${CMAKE_LIBRARY_OUTPUT_DIRECTORY}
#               -DCMAKE_INSTALL_INCLUDEDIR=${CMAKE_LIBRARY_INCLUDE_DIRECTORY}
#               -DCMAKE_INSTALL_PREFIX=${CMAKE_PREFIX_PATH}
#               -DCMAKE_PREFIX_PATH=${CMAKE_PREFIX_PATH}
#)

include_directories(../src/ ../src/utility/)
add_executable(${PROJECT_NAME} parser.cpp ${SOURCES})
#target_link_libraries(${PROJECT_NAME} ${LIB}libgtest_main.a ${LIB}libgtest.a ${LIB}libgmock_main.a ${LIB}libgmock.a)
#target_link_libraries(${PROJECT_NAME} ${LIB}libgtest.a ${LIB}libgtest_main.a pthread)
target_link_libraries(${PROJECT_NAME} gtest gtest_main pthread)
