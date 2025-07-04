set(CMAKE_CXX_STANDARD 17)

option(BASE64_ENABLE_ASAN "build asan for unittests" ON)

if(NOT CMAKE_HOST_SYSTEM_NAME MATCHES "Windows" AND MSVC)
  set(gtest_disable_pthreads ON)
endif()

include(FetchContent)
FetchContent_Declare(
  googletest
  GIT_REPOSITORY https://github.com/google/googletest.git
  GIT_TAG v1.15.2
  GIT_SHALLOW TRUE)
FetchContent_MakeAvailable(googletest)

FetchContent_Declare(
  subprocess
  GIT_REPOSITORY https://github.com/shediao/subprocess.hpp
  GIT_TAG v0.0.5)
FetchContent_MakeAvailable(subprocess)

FetchContent_Declare(
  environment
  GIT_REPOSITORY https://github.com/shediao/environment.hpp
  GIT_TAG e0c79afb6944aee02f5acf7037ed6393be80ff4a)
FetchContent_MakeAvailable(environment)

# --- Compiler and Platform Specific Configuration ---

# Define common compile options based on the compiler.
set(BASE64_MSVC_COMPILE_OPTIONS "/W4" "/WX" "/utf-8" "/EHsc")
set(BASE64_GCC_CLANG_COMPILE_OPTIONS "-Wall" "-Wextra" "-Werror")

if(MSVC)
  set(BASE64_COMMON_COMPILE_OPTIONS ${BASE64_MSVC_COMPILE_OPTIONS})
else()
  set(BASE64_COMMON_COMPILE_OPTIONS ${BASE64_GCC_CLANG_COMPILE_OPTIONS})
endif()

# Configure sanitizer flags for Debug builds on Linux and macOS when not
# cross-compiling.
set(BASE64_SANITIZER_CXX_FLAGS "")
set(BASE64_SANITIZER_LINK_FLAGS "")
if(BASE64_ENABLE_ASAN)
  if(NOT CMAKE_CROSSCOMPILING AND CMAKE_HOST_SYSTEM_NAME MATCHES "Linux|Darwin")
    if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang|AppleClang")
      list(
        APPEND
        BASE64_SANITIZER_CXX_FLAGS
        "-fsanitize=address,undefined"
        "-fno-omit-frame-pointer"
        "-fno-optimize-sibling-calls"
        "-g"
        "-O0")
      list(APPEND BASE64_SANITIZER_LINK_FLAGS "-fsanitize=address,undefined")
    endif()
    if(CMAKE_CXX_COMPILER_ID MATCHES "MSVC")
      list(APPEND BASE64_SANITIZER_CXX_FLAGS "/fsanitize=address")
    endif()
  endif()
endif()

# --- Helper Function for Creating Tests ---

# A helper function to create test targets. It handles the logic for creating
# standard and, on Windows, ANSI targets.
function(add_test_suite test_name)
  set(sources ${ARGN})
  set(variants "")
  if(CMAKE_SYSTEM_NAME STREQUAL "Windows")
    set(variants "unicode" "ansi")
  else()
    set(variants "default")
  endif()

  foreach(variant IN LISTS variants)
    set(target_name ${test_name})
    set(test_name_suffix "")

    if(variant STREQUAL "ansi")
      set(target_name "${test_name}_ansi")
      set(test_name_suffix "_ansi")
    endif()

    # Add the executable target
    add_executable(${target_name} ${sources})

    # Set compile options
    target_compile_options(${target_name}
                           PRIVATE ${BASE64_COMMON_COMPILE_OPTIONS})
    if(BASE64_SANITIZER_CXX_FLAGS)
      target_compile_options(
        ${target_name}
        PRIVATE "$<$<CONFIG:Debug>:${BASE64_SANITIZER_CXX_FLAGS}>")
    endif()
    if(BASE64_SANITIZER_LINK_FLAGS)
      target_link_options(${target_name} PRIVATE
                          "$<$<CONFIG:Debug>:${BASE64_SANITIZER_LINK_FLAGS}>")
    endif()

    # Set compile definitions for the UNICODE variant on Windows
    if(variant STREQUAL "unicode")
      target_compile_definitions(${target_name} PRIVATE UNICODE _UNICODE)
    endif()

    # Link libraries
    target_link_libraries(
      ${target_name} PRIVATE base64::base64 GTest::gtest GTest::gtest_main
                             subprocess::subprocess environment::environment)

    # Add to CTest
    add_test(NAME ${test_name}${test_name_suffix} COMMAND ${target_name})
  endforeach()
endfunction()

# --- Test Target Definitions ---

# Glob all test source files.
file(
  GLOB test_sources
  RELATIVE ${CMAKE_CURRENT_SOURCE_DIR}
  "*.cc")

# Create a separate test executable for each source file.
foreach(test_source IN LISTS test_sources)
  get_filename_component(test_name ${test_source} NAME_WE)
  add_test_suite(${test_name} ${test_source})
endforeach()

# Create a single test executable containing all tests.
add_test_suite(all_tests ${test_sources})
