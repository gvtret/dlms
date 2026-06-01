cmake_minimum_required(VERSION 3.16)

include("${CMAKE_CURRENT_SOURCE_DIR}/../../cmake/DlmsVersion.cmake")

project(dlms_llc
  VERSION ${DLMS_PROJECT_VERSION}
  LANGUAGES C CXX)

option(DLMS_BUILD_TESTS "Build GoogleTest tests" ON)
option(DLMS_BUILD_C_API "Build stable C ABI wrapper" ON)
option(DLMS_USE_SYSTEM_GTEST "Use system-installed GoogleTest" OFF)

add_library(dlms_llc
  src/llc/llc_codec.cpp
  src/llc/llc_header.cpp)

if(DLMS_BUILD_C_API)
  target_sources(dlms_llc
    PRIVATE
      src/llc/llc_c_api.cpp)
endif()

target_compile_features(dlms_llc PUBLIC cxx_std_11)

target_include_directories(dlms_llc
  PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>)

set(DLMS_LLC_HAS_CODEC_API ON CACHE INTERNAL
  "dlms-llc exposes the codec API required by root integration tests")

if(DLMS_BUILD_TESTS)
  enable_testing()
  add_subdirectory(test)
endif()
