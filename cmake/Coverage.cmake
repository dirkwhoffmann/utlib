# -----------------------------------------------------------------------------
# Code coverage support
#
# Enable with -DUTLIB_COVERAGE=ON. This instruments the library and the test
# binary, and adds a 'coverage' target that runs the tests and reports how much
# of utlib they actually reach:
#
#   cmake -B build -DUTLIB_COVERAGE=ON
#   cmake --build build --target coverage
#
# Coverage is off by default. The instrumentation slows the binary down and
# changes what is generated, so it is not something an ordinary Debug build
# should carry.
# -----------------------------------------------------------------------------

option(UTLIB_COVERAGE "Build with code coverage instrumentation" OFF)

if(NOT UTLIB_COVERAGE)
  return()
endif()

if(NOT CMAKE_BUILD_TYPE STREQUAL "Debug")
  message(WARNING
    "UTLIB_COVERAGE is on but CMAKE_BUILD_TYPE is '${CMAKE_BUILD_TYPE}'. "
    "Optimised builds inline and fold code away, so the line counts will not "
    "reflect the source. Use -DCMAKE_BUILD_TYPE=Debug.")
endif()

# Everything below the source root except the tests themselves and vendored
# code -- those would only dilute the number we care about.
set(UTLIB_COVERAGE_EXCLUDE "(unittests|thirdparty)/"
    CACHE STRING "Regex of paths to leave out of the coverage report")

if(CMAKE_CXX_COMPILER_ID MATCHES "Clang")

  #
  # Clang: source-based coverage
  #
  # Preferred over gcov-style counters because it maps regions rather than
  # lines, so a partially executed line (a short-circuited condition, say) is
  # reported as such instead of being counted as covered.
  #

  set(UTLIB_COVERAGE_FLAGS -fprofile-instr-generate -fcoverage-mapping)

  # On Apple platforms the tools live inside the active Xcode toolchain rather
  # than on PATH, so ask xcrun before falling back to a plain lookup.
  find_program(XCRUN_EXECUTABLE xcrun)
  if(XCRUN_EXECUTABLE)
    execute_process(COMMAND ${XCRUN_EXECUTABLE} --find llvm-profdata
                    OUTPUT_VARIABLE LLVM_PROFDATA OUTPUT_STRIP_TRAILING_WHITESPACE
                    ERROR_QUIET)
    execute_process(COMMAND ${XCRUN_EXECUTABLE} --find llvm-cov
                    OUTPUT_VARIABLE LLVM_COV OUTPUT_STRIP_TRAILING_WHITESPACE
                    ERROR_QUIET)
  endif()

  if(NOT LLVM_PROFDATA)
    find_program(LLVM_PROFDATA NAMES llvm-profdata)
  endif()
  if(NOT LLVM_COV)
    find_program(LLVM_COV NAMES llvm-cov)
  endif()

  if(NOT LLVM_PROFDATA OR NOT LLVM_COV)
    message(FATAL_ERROR
      "UTLIB_COVERAGE needs llvm-profdata and llvm-cov, which were not found. "
      "Install the LLVM tools, or configure with -DUTLIB_COVERAGE=OFF.")
  endif()

  set(UTLIB_COVERAGE_DIR ${CMAKE_BINARY_DIR}/coverage)

  # The profile has a fixed name rather than the usual %p pattern: these
  # commands run without a shell, so a glob would reach llvm-profdata as a
  # literal '*'. Wiping the directory first is what keeps a stale profile from
  # a crashed run out of the merge.
  add_custom_target(coverage
    COMMAND ${CMAKE_COMMAND} -E rm -rf ${UTLIB_COVERAGE_DIR}
    COMMAND ${CMAKE_COMMAND} -E make_directory ${UTLIB_COVERAGE_DIR}

    COMMAND ${CMAKE_COMMAND} -E env
            LLVM_PROFILE_FILE=${UTLIB_COVERAGE_DIR}/utlib.profraw
            $<TARGET_FILE:unittester>

    COMMAND ${LLVM_PROFDATA} merge -sparse
            ${UTLIB_COVERAGE_DIR}/utlib.profraw
            -o ${UTLIB_COVERAGE_DIR}/utlib.profdata

    COMMAND ${LLVM_COV} report $<TARGET_FILE:unittester>
            -instr-profile=${UTLIB_COVERAGE_DIR}/utlib.profdata
            -ignore-filename-regex=${UTLIB_COVERAGE_EXCLUDE}

    COMMAND ${LLVM_COV} show $<TARGET_FILE:unittester>
            -instr-profile=${UTLIB_COVERAGE_DIR}/utlib.profdata
            -ignore-filename-regex=${UTLIB_COVERAGE_EXCLUDE}
            -format=html
            -output-dir=${UTLIB_COVERAGE_DIR}/html

    COMMAND ${CMAKE_COMMAND} -E echo
            "HTML report: ${UTLIB_COVERAGE_DIR}/html/index.html"

    DEPENDS unittester
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
    COMMENT "Running the test suite and measuring coverage"
    USES_TERMINAL VERBATIM)

elseif(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")

  #
  # GCC: gcov-style counters
  #

  set(UTLIB_COVERAGE_FLAGS --coverage)

  find_program(GCOVR_EXECUTABLE gcovr)

  if(GCOVR_EXECUTABLE)

    set(UTLIB_COVERAGE_DIR ${CMAKE_BINARY_DIR}/coverage)

    add_custom_target(coverage
      COMMAND ${CMAKE_COMMAND} -E make_directory ${UTLIB_COVERAGE_DIR}
      COMMAND $<TARGET_FILE:unittester>
      COMMAND ${GCOVR_EXECUTABLE}
              --root ${CMAKE_SOURCE_DIR}
              --exclude "${CMAKE_SOURCE_DIR}/unittests"
              --exclude "${CMAKE_SOURCE_DIR}/thirdparty"
              --html-details ${UTLIB_COVERAGE_DIR}/index.html
              --print-summary
      DEPENDS unittester
      WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
      COMMENT "Running the test suite and measuring coverage"
      USES_TERMINAL VERBATIM)

  else()

    # Still instrument, so the .gcda files are there to inspect by hand
    message(WARNING
      "gcovr was not found, so no 'coverage' target is available. The build is "
      "still instrumented -- run the tests and use gcov on the .gcda files, or "
      "install gcovr for a report.")

  endif()

else()

  message(FATAL_ERROR
    "UTLIB_COVERAGE is not supported for compiler '${CMAKE_CXX_COMPILER_ID}'. "
    "Use Clang or GCC.")

endif()

# Applies the instrumentation to a target. Both compiling and linking need the
# flags: the counters are emitted per object, and the runtime that writes the
# profile out at exit is pulled in at link time.
function(utlib_enable_coverage target)
  target_compile_options(${target} PRIVATE ${UTLIB_COVERAGE_FLAGS})
  target_link_options(${target} PRIVATE ${UTLIB_COVERAGE_FLAGS})
endfunction()
