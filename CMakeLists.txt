cmake_minimum_required(VERSION 3.10)
project(arcana
  LANGUAGES C CXX
  VERSION 0.1.0
  DESCRIPTION "language framework"
)

add_library(
  arcana SHARED
  src/token.c
  src/table.c
)

find_package(GTest REQUIRED)
enable_testing()
add_executable(
  unit

  tests/monkey.cpp
  tests/tokenize.cpp
)

target_include_directories(unit PUBLIC ./src)
target_link_libraries(unit GTest::gtest_main)
target_link_libraries(unit arcana)

add_test(
  NAME unit-tests 
  COMMAND unit
  WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
)
