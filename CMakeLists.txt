cmake_minimum_required(VERSION 3.15)
project(
  base64
  VERSION "0.1"
  LANGUAGES CXX)

set("PROJECT_DESCRIPTION" "A header-only base64 library")
set("PROJECT_HOMEPAGE_URL" "https://github.com/shediao/base64.hpp")

if(CMAKE_PROJECT_NAME STREQUAL PROJECT_NAME)
  option(BASE64_BUILD_TESTS "Set to ON to build tests" ON)
else()
  option(BASE64_BUILD_TESTS "Set to OFF to build tests" OFF)
endif()

add_library(base64 INTERFACE)

target_include_directories(
  base64 INTERFACE $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
                   $<INSTALL_INTERFACE:include>)

target_compile_features(base64 INTERFACE cxx_std_20)
add_library(base64::base64 ALIAS base64)

if(BASE64_BUILD_TESTS)
  enable_testing()
  add_subdirectory(tests)
endif()

install(
  TARGETS base64
  EXPORT base64-targets
  INCLUDES
  DESTINATION include)

install(
  DIRECTORY include/
  DESTINATION include
  FILES_MATCHING
  PATTERN "*.hpp")

install(
  EXPORT base64-targets
  FILE base64-targets.cmake
  NAMESPACE base64::
  DESTINATION lib/cmake/base64)
