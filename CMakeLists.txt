cmake_minimum_required(VERSION 3.14)
project(MineCompiler)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)


include(FetchContent)

FetchContent_Declare(
  Catch2
  GIT_REPOSITORY https://github.com/catchorg/Catch2.git
  GIT_TAG        v3.4.0 
)

FetchContent_MakeAvailable(Catch2)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++20")

file(GLOB_RECURSE LIB_SOURCES src/*.cpp)
list(FILTER LIB_SOURCES EXCLUDE REGEX "src/main.cpp") 

add_library(compiler_logic STATIC ${LIB_SOURCES})

add_executable(basic src/main.cpp)
target_link_libraries(basic PRIVATE compiler_logic)

file(GLOB_RECURSE TEST_SOURCES tests/*.cpp)
add_executable(unit_tests ${TEST_SOURCES})
target_link_libraries(unit_tests PRIVATE compiler_logic Catch2::Catch2WithMain)
