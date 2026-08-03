cmake_minimum_required(VERSION 3.20 FATAL_ERROR)

project(
  utlib
  VERSION 0.1
  DESCRIPTION "A lightweight C++ utility library"
  HOMEPAGE_URL "https://www.dirkwhoffmann.de"
  LANGUAGES CXX C)

# Set the C++ standard
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_CXX_EXTENSIONS False)

# Default to a Debug build, so the unit tests run with assertions enabled
if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Debug)
endif()

# Optional code coverage instrumentation (see cmake/Coverage.cmake)
include(cmake/Coverage.cmake)

add_library(utlib)

target_include_directories(utlib PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
        ${CMAKE_CURRENT_SOURCE_DIR}/thirdparty)

target_compile_features(utlib PUBLIC cxx_std_20)

file(GLOB_RECURSE UTL_SRC
        "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/*.c")

target_sources(utlib PRIVATE ${UTL_SRC})

if(UTLIB_COVERAGE)
  utlib_enable_coverage(utlib)
endif()

# Only build the tests when utlib is the top-level project, so that adding this
# repository to a consumer (e.g. via add_subdirectory) does not drag them in.
if(PROJECT_IS_TOP_LEVEL)
  enable_testing()
  add_subdirectory(unittests)
endif()
