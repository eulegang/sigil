cmake_minimum_required(VERSION 3.10)
project(sigil
  LANGUAGES C CXX
  VERSION 0.1.0
  DESCRIPTION "language framework"
)

add_library(
  sigil SHARED
  src/tokens.c
  src/table.c
  src/util.c
  src/slice.c
  src/parse.c
  src/ast.c
  src/state.c
  src/overlay.c
)

find_package(GTest REQUIRED)
enable_testing()
add_executable(
  unit

  tests/monkey.cpp
  tests/monkey_parse.cpp
  tests/monkey_debug.cpp
  tests/tokens.cpp
  tests/parser.cpp
  tests/util.cpp
)

target_include_directories(unit PUBLIC ./src)
target_link_libraries(unit GTest::gtest_main)
target_link_libraries(unit sigil)

add_test(
  NAME unit-tests 
  COMMAND unit
  WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
)

configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/src/sigil.pc.in
    ${CMAKE_CURRENT_BINARY_DIR}/sigil.pc
    @ONLY
)

install(TARGETS sigil RUNTIME DESTINATION lib)
install(FILES src/sigil.h DESTINATION include)
install(FILES "${CMAKE_CURRENT_BINARY_DIR}/sigil.pc" DESTINATION lib/pkgconfig)
