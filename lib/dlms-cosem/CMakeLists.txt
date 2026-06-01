cmake_minimum_required(VERSION 3.16)

include("${CMAKE_CURRENT_SOURCE_DIR}/../../cmake/DlmsVersion.cmake")

project(dlms_cosem
  VERSION ${DLMS_PROJECT_VERSION}
  LANGUAGES CXX)

option(DLMS_BUILD_TESTS "Build GoogleTest tests" ON)
option(DLMS_USE_SYSTEM_GTEST "Use system-installed GoogleTest" OFF)

add_library(dlms_cosem
  src/cosem/cosem_access.cpp
  src/cosem/cosem_status.cpp
  src/cosem/cosem_types.cpp
  src/cosem/logical_device.cpp
  src/cosem/object_registry.cpp
  src/cosem/simple_objects.cpp)

target_compile_features(dlms_cosem PUBLIC cxx_std_11)

target_include_directories(dlms_cosem
  PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>)

set(DLMS_COSEM_HAS_OBJECT_MODEL_API ON CACHE INTERNAL
  "dlms-cosem exposes the COSEM object model API")

if(DLMS_BUILD_TESTS)
  enable_testing()
  add_subdirectory(test)
endif()
