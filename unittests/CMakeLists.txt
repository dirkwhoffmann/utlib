cmake_minimum_required(VERSION 3.20 FATAL_ERROR)

add_executable(unittester

    main.cpp
    smoke_tests.cpp
    uuid_tests.cpp
    strings_tests.cpp
    bits_tests.cpp
    chrono_tests.cpp
    files_tests.cpp
    storage_tests.cpp
)

target_link_libraries(unittester PRIVATE utlib)
target_include_directories(unittester PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

# Specify compile options
if(MSVC)

  target_compile_options(unittester PUBLIC /W4)
  target_compile_options(unittester PUBLIC /wd4244)

else()

  target_compile_options(unittester PUBLIC -Wall -Werror -Wno-unknown-pragmas)

endif()

if(UTLIB_COVERAGE)
  utlib_enable_coverage(unittester)
endif()

add_test(NAME unittester COMMAND unittester)
