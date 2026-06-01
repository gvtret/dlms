cmake_minimum_required(VERSION 3.16)

include("${CMAKE_CURRENT_SOURCE_DIR}/../../cmake/DlmsVersion.cmake")

project(dlms_endpoint
  VERSION ${DLMS_PROJECT_VERSION}
  LANGUAGES CXX)

option(DLMS_BUILD_TESTS "Build GoogleTest tests" ON)
option(DLMS_USE_SYSTEM_GTEST "Use system-installed GoogleTest" OFF)

add_library(dlms_endpoint
  src/endpoint/client_endpoint.cpp
  src/endpoint/endpoint_factories.cpp
  src/endpoint/gateway_endpoint.cpp
  src/endpoint/listener_runtime.cpp
  src/endpoint/endpoint_options.cpp
  src/endpoint/push_listener_endpoint.cpp
  src/endpoint/server_endpoint.cpp
  src/endpoint/endpoint_status.cpp)

set(DLMS_ENDPOINT_BUILD_TESTS ${DLMS_BUILD_TESTS})
set(DLMS_BUILD_TESTS OFF CACHE BOOL
  "Build dependency tests from dlms-endpoint" FORCE)

if(NOT TARGET dlms_transport AND EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/../dlms-transport/CMakeLists.txt")
  add_subdirectory("${CMAKE_CURRENT_SOURCE_DIR}/../dlms-transport" "${CMAKE_CURRENT_BINARY_DIR}/_deps/dlms-transport")
endif()

if(NOT TARGET dlms_profile AND EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/../dlms-profile/CMakeLists.txt")
  add_subdirectory("${CMAKE_CURRENT_SOURCE_DIR}/../dlms-profile" "${CMAKE_CURRENT_BINARY_DIR}/_deps/dlms-profile")
endif()

if(NOT TARGET dlms_security AND EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/../dlms-security/CMakeLists.txt")
  add_subdirectory("${CMAKE_CURRENT_SOURCE_DIR}/../dlms-security" "${CMAKE_CURRENT_BINARY_DIR}/_deps/dlms-security")
endif()

if(NOT TARGET dlms_client AND EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/../dlms-client/CMakeLists.txt")
  add_subdirectory("${CMAKE_CURRENT_SOURCE_DIR}/../dlms-client" "${CMAKE_CURRENT_BINARY_DIR}/_deps/dlms-client")
endif()

if(NOT TARGET dlms_server AND EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/../dlms-server/CMakeLists.txt")
  add_subdirectory("${CMAKE_CURRENT_SOURCE_DIR}/../dlms-server" "${CMAKE_CURRENT_BINARY_DIR}/_deps/dlms-server")
endif()

set(DLMS_BUILD_TESTS ${DLMS_ENDPOINT_BUILD_TESTS} CACHE BOOL
  "Build GoogleTest tests" FORCE)
add_library(dlms::endpoint ALIAS dlms_endpoint)

target_compile_features(dlms_endpoint PUBLIC cxx_std_11)

target_include_directories(dlms_endpoint
  PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include)

target_link_libraries(dlms_endpoint
  PUBLIC
    dlms_association
    dlms_client
    dlms_profile
    dlms_security
    dlms_server
    dlms_transport)

set(DLMS_ENDPOINT_HAS_OPTIONS_API ON CACHE INTERNAL
  "dlms-endpoint exposes status and option validation contracts")

if(DLMS_ENDPOINT_BUILD_TESTS)
  enable_testing()
  add_subdirectory(test)
endif()
