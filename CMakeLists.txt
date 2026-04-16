cmake_minimum_required(VERSION 3.10)
project(arcana
  LANGUAGES C CXX
  VERSION 0.1.0
  DESCRIPTION "language framework"
)

add_library(
  arcana SHARED
  src/tokens.c
  src/table.c
)

find_package(GTest REQUIRED)
enable_testing()
add_executable(
  unit

  tests/monkey.cpp
  tests/tokens.cpp
  tests/util.cpp
)

target_include_directories(unit PUBLIC ./src)
target_link_libraries(unit GTest::gtest_main)
target_link_libraries(unit arcana)

add_test(
  NAME unit-tests 
  COMMAND unit
  WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
)

configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/src/arcana.pc.in
    ${CMAKE_CURRENT_BINARY_DIR}/arcana.pc
    @ONLY
)

install(TARGETS arcana RUNTIME DESTINATION lib)
install(FILES src/arcana.h DESTINATION include)
install(FILES "${CMAKE_CURRENT_BINARY_DIR}/arcana.pc" DESTINATION lib/pkgconfig)
