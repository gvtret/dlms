if(NOT DEFINED BINARY_DIR)
  message(FATAL_ERROR "BINARY_DIR is required")
endif()

if(NOT DEFINED SOURCE_DIR)
  message(FATAL_ERROR "SOURCE_DIR is required")
endif()

if(NOT DEFINED GENERATOR)
  message(FATAL_ERROR "GENERATOR is required")
endif()

set(SMOKE_DIR "${BINARY_DIR}/package-smoke")
set(INSTALL_PREFIX "${SMOKE_DIR}/install")
set(CONSUMER_DIR "${SMOKE_DIR}/consumer")
set(CONSUMER_BUILD_DIR "${SMOKE_DIR}/consumer-build")
set(CODEC_CONSUMER_DIR "${SMOKE_DIR}/codec-consumer")
set(CODEC_CONSUMER_BUILD_DIR "${SMOKE_DIR}/codec-consumer-build")
set(IO_CONSUMER_DIR "${SMOKE_DIR}/io-consumer")
set(IO_CONSUMER_BUILD_DIR "${SMOKE_DIR}/io-consumer-build")
set(EXAMPLES_BUILD_DIR "${SMOKE_DIR}/examples-build")

file(REMOVE_RECURSE "${SMOKE_DIR}")
file(MAKE_DIRECTORY "${CONSUMER_DIR}")
file(MAKE_DIRECTORY "${CODEC_CONSUMER_DIR}")
file(MAKE_DIRECTORY "${IO_CONSUMER_DIR}")

execute_process(
  COMMAND "${CMAKE_COMMAND}" --install "${BINARY_DIR}" --prefix "${INSTALL_PREFIX}"
  RESULT_VARIABLE install_result)
if(NOT install_result EQUAL 0)
  message(FATAL_ERROR "DLMSFramework install smoke failed during install")
endif()

set(CONFIG_FILE "${INSTALL_PREFIX}/lib/cmake/DLMSFramework/DLMSFrameworkConfig.cmake")
set(TARGETS_FILE "${INSTALL_PREFIX}/lib/cmake/DLMSFramework/DLMSFrameworkTargets.cmake")

foreach(required_file
    "${CONFIG_FILE}"
    "${TARGETS_FILE}")
  if(NOT EXISTS "${required_file}")
    message(FATAL_ERROR "DLMSFramework install smoke missing CMake file: ${required_file}")
  endif()
endforeach()

# CMake emits per-config target files named after the build type:
#   single-config (Make/Ninja, Debug)      -> DLMSFrameworkTargets-debug.cmake
#   single-config (Make/Ninja, Release)    -> DLMSFrameworkTargets-release.cmake
#   multi-config (VS/Xcode), no config set -> DLMSFrameworkTargets-noconfig.cmake
# The smoke contract is "at least one per-config targets file is installed";
# pick whichever the current build produced.
file(GLOB TARGETS_PER_CONFIG_FILES
  "${INSTALL_PREFIX}/lib/cmake/DLMSFramework/DLMSFrameworkTargets-*.cmake")
if(NOT TARGETS_PER_CONFIG_FILES)
  message(FATAL_ERROR
    "DLMSFramework install smoke missing per-config CMake file: "
    "${INSTALL_PREFIX}/lib/cmake/DLMSFramework/DLMSFrameworkTargets-*.cmake")
endif()
list(GET TARGETS_PER_CONFIG_FILES 0 TARGETS_NOCONFIG_FILE)

file(READ "${CONFIG_FILE}" config_contents)
if(NOT config_contents MATCHES "find_dependency\\(OpenSSL\\)")
  message(FATAL_ERROR "DLMSFrameworkConfig.cmake does not declare OpenSSL dependency")
endif()

file(READ "${TARGETS_FILE}" targets_contents)
file(READ "${TARGETS_NOCONFIG_FILE}" targets_noconfig_contents)
foreach(required_export
    "dlms::codec"
    "dlms::io"
    "dlms::protocol"
    "dlms::cosem_server"
    "dlms::runtime"
    "dlms::framework")
  if(NOT targets_contents MATCHES "${required_export}")
    message(FATAL_ERROR "DLMSFrameworkTargets.cmake missing export: ${required_export}")
  endif()
endforeach()

if(NOT targets_contents MATCHES "INTERFACE_INCLUDE_DIRECTORIES")
  message(FATAL_ERROR "DLMSFrameworkTargets.cmake does not export include directories")
endif()

foreach(expected_link IN ITEMS
    [=[INTERFACE_LINK_LIBRARIES "dlms::dlms_hdlc;dlms::dlms_llc;dlms::dlms_wrapper;dlms::dlms_apdu"]=]
    [=[INTERFACE_LINK_LIBRARIES "dlms::dlms_transport;dlms::dlms_profile"]=]
    [=[INTERFACE_LINK_LIBRARIES "dlms::dlms_association;dlms::dlms_security;dlms::dlms_xdlms"]=]
    [=[INTERFACE_LINK_LIBRARIES "dlms::dlms_cosem;dlms::dlms_server"]=]
    [=[INTERFACE_LINK_LIBRARIES "dlms::dlms_client;dlms::dlms_endpoint"]=]
    [=[INTERFACE_LINK_LIBRARIES "dlms::codec;dlms::io;dlms::protocol;dlms::cosem_server;dlms::runtime"]=])
  if(NOT targets_contents MATCHES "${expected_link}")
    message(FATAL_ERROR
      "DLMSFrameworkTargets.cmake missing aggregate link interface: ${expected_link}")
  endif()
endforeach()

foreach(disallowed_pattern
    "gtest"
    "gmock"
    "GTest::")
  if(targets_contents MATCHES "${disallowed_pattern}" OR
     targets_noconfig_contents MATCHES "${disallowed_pattern}")
    message(FATAL_ERROR
      "DLMSFramework exported targets contain test dependency pattern: ${disallowed_pattern}")
  endif()
endforeach()

# Config.cmake.in OpenSSL gating audit (P1 Package §2).
#
# Contract:
#   codec and io aggregates do NOT depend on OpenSSL, so a consumer that
#   pulls only those components must not require OpenSSL on the build host.
#   protocol, cosem_server, runtime, framework DO depend on dlms_security
#   and therefore must trigger find_dependency(OpenSSL).
#
# This protects against future regressions if someone extends the aggregate
# composition or the gating logic in DLMSFrameworkConfig.cmake.in.
file(READ "${CONFIG_FILE}" config_contents)

string(REGEX MATCH
  "if[(]_DLMSFramework_requires_OpenSSL[)]"
  config_has_gate
  "${config_contents}")
if(config_has_gate STREQUAL "")
  message(FATAL_ERROR
    "DLMSFrameworkConfig.cmake missing OpenSSL gating block")
endif()

foreach(opt_in_component IN ITEMS "protocol" "cosem_server" "runtime" "framework")
  string(REGEX MATCH
    "STREQUAL \"${opt_in_component}\""
    component_marked
    "${config_contents}")
  if(component_marked STREQUAL "")
    message(FATAL_ERROR
      "DLMSFrameworkConfig.cmake does not enable OpenSSL for component '${opt_in_component}'")
  endif()
endforeach()

foreach(opt_out_component IN ITEMS "codec" "io")
  string(REGEX MATCH
    "STREQUAL \"${opt_out_component}\""
    component_marked
    "${config_contents}")
  if(NOT component_marked STREQUAL "")
    message(FATAL_ERROR
      "DLMSFrameworkConfig.cmake unexpectedly enables OpenSSL for component '${opt_out_component}'")
  endif()
endforeach()

file(WRITE "${CONSUMER_DIR}/CMakeLists.txt" [=[
cmake_minimum_required(VERSION 3.16)
project(dlms_package_consumer LANGUAGES CXX)

find_package(DLMSFramework REQUIRED CONFIG
  COMPONENTS
    codec
    io
    protocol
    cosem_server
    runtime
    framework)

foreach(required_target
    dlms::codec
    dlms::io
    dlms::protocol
    dlms::cosem_server
    dlms::runtime
    dlms::framework)
  if(NOT TARGET ${required_target})
    message(FATAL_ERROR "Required DLMSFramework target is missing: ${required_target}")
  endif()
endforeach()

add_executable(dlms_package_consumer main.cpp)
target_compile_features(dlms_package_consumer PRIVATE cxx_std_11)
target_link_libraries(dlms_package_consumer
  PRIVATE
    dlms::codec
    dlms::io
    dlms::protocol
    dlms::cosem_server
    dlms::runtime
    dlms::framework)
]=])

file(WRITE "${CONSUMER_DIR}/main.cpp" [=[
#include "dlms/apdu/apdu_types.hpp"
#include "dlms/client/client_xdlms_service_interface.hpp"
#include "dlms/cosem/logical_device_interface.hpp"
#include "dlms/endpoint/apdu_channel_listener.hpp"
#include "dlms/endpoint/gateway_interfaces.hpp"
#include "dlms/endpoint/push_indication_handler.hpp"
#include "dlms/server/server_service_interface.hpp"
#include "dlms/transport/transport_status.hpp"
#include "dlms/xdlms/xdlms_association_state_interface.hpp"
#include "dlms/xdlms/xdlms_security_processor_interface.hpp"

#include <cstddef>
#include <cstdint>
#include <type_traits>

int main()
{
  static_assert(std::is_polymorphic<dlms::client::IClientXdlmsService>::value,
    "client xDLMS service interface must be polymorphic");
  static_assert(std::is_polymorphic<dlms::cosem::ILogicalDevice>::value,
    "COSEM logical-device interface must be polymorphic");
  static_assert(std::is_polymorphic<dlms::endpoint::IApduChannelListener>::value,
    "endpoint APDU listener interface must be polymorphic");
  static_assert(std::is_polymorphic<dlms::endpoint::IGatewayPolicy>::value,
    "endpoint gateway policy interface must be polymorphic");
  static_assert(std::is_polymorphic<dlms::endpoint::IPushIndicationHandler>::value,
    "endpoint push handler interface must be polymorphic");
  static_assert(std::is_polymorphic<dlms::server::IServerService>::value,
    "server service interface must be polymorphic");
  static_assert(std::is_polymorphic<dlms::xdlms::IXdlmsAssociationState>::value,
    "xDLMS association-state interface must be polymorphic");
  static_assert(std::is_polymorphic<dlms::xdlms::IXdlmsSecurityProcessor>::value,
    "xDLMS security processor interface must be polymorphic");

  const dlms::apdu::ByteView empty = {
    static_cast<const std::uint8_t*>(0),
    static_cast<std::size_t>(0)
  };

  if (empty.data != 0 || empty.size != 0) {
    return 1;
  }

  return dlms::transport::ToString(dlms::transport::TransportStatus::Ok)[0] == 'O'
    ? 0
    : 1;
}
]=])

execute_process(
  COMMAND "${CMAKE_COMMAND}" -S "${CONSUMER_DIR}" -B "${CONSUMER_BUILD_DIR}"
    -G "${GENERATOR}"
    "-DDLMSFramework_DIR=${INSTALL_PREFIX}/lib/cmake/DLMSFramework"
  RESULT_VARIABLE configure_result)
if(NOT configure_result EQUAL 0)
  message(FATAL_ERROR "DLMSFramework install smoke failed during consumer configure")
endif()

execute_process(
  COMMAND "${CMAKE_COMMAND}" --build "${CONSUMER_BUILD_DIR}"
  RESULT_VARIABLE build_result)
if(NOT build_result EQUAL 0)
  message(FATAL_ERROR "DLMSFramework install smoke failed during consumer build")
endif()

file(WRITE "${CODEC_CONSUMER_DIR}/CMakeLists.txt" [=[
cmake_minimum_required(VERSION 3.16)
project(dlms_codec_only_consumer LANGUAGES CXX)

find_package(DLMSFramework REQUIRED CONFIG COMPONENTS codec)

if(NOT TARGET dlms::codec)
  message(FATAL_ERROR "Required DLMSFramework target is missing: dlms::codec")
endif()

add_executable(dlms_codec_only_consumer main.cpp)
target_compile_features(dlms_codec_only_consumer PRIVATE cxx_std_11)
target_link_libraries(dlms_codec_only_consumer PRIVATE dlms::codec)
]=])

file(WRITE "${CODEC_CONSUMER_DIR}/main.cpp" [=[
#include "dlms/llc/llc_header.hpp"

int main()
{
  const dlms::llc::LlcHeader header =
    dlms::llc::MakeLlcHeader(dlms::llc::LlcDirection::ClientToServer);
  return header.dsap == 0xE6 && header.ssap == 0xE6 && header.control == 0x00
    ? 0
    : 1;
}
]=])

execute_process(
  COMMAND "${CMAKE_COMMAND}" -S "${CODEC_CONSUMER_DIR}" -B "${CODEC_CONSUMER_BUILD_DIR}"
    -G "${GENERATOR}"
    "-DDLMSFramework_DIR=${INSTALL_PREFIX}/lib/cmake/DLMSFramework"
    "-DCMAKE_DISABLE_FIND_PACKAGE_OpenSSL=TRUE"
  RESULT_VARIABLE codec_configure_result)
if(NOT codec_configure_result EQUAL 0)
  message(FATAL_ERROR
    "DLMSFramework install smoke failed during codec-only consumer configure")
endif()

execute_process(
  COMMAND "${CMAKE_COMMAND}" --build "${CODEC_CONSUMER_BUILD_DIR}"
  RESULT_VARIABLE codec_build_result)
if(NOT codec_build_result EQUAL 0)
  message(FATAL_ERROR
    "DLMSFramework install smoke failed during codec-only consumer build")
endif()

file(WRITE "${IO_CONSUMER_DIR}/CMakeLists.txt" [=[
cmake_minimum_required(VERSION 3.16)
project(dlms_io_only_consumer LANGUAGES CXX)

find_package(DLMSFramework REQUIRED CONFIG COMPONENTS io)

if(NOT TARGET dlms::io)
  message(FATAL_ERROR "Required DLMSFramework target is missing: dlms::io")
endif()

add_executable(dlms_io_only_consumer main.cpp)
target_compile_features(dlms_io_only_consumer PRIVATE cxx_std_11)
target_link_libraries(dlms_io_only_consumer PRIVATE dlms::io)
]=])

file(WRITE "${IO_CONSUMER_DIR}/main.cpp" [=[
#include "dlms/transport/transport_status.hpp"

int main()
{
  return dlms::transport::ToString(dlms::transport::TransportStatus::Ok)[0] == 'O'
    ? 0
    : 1;
}
]=])

execute_process(
  COMMAND "${CMAKE_COMMAND}" -S "${IO_CONSUMER_DIR}" -B "${IO_CONSUMER_BUILD_DIR}"
    -G "${GENERATOR}"
    "-DDLMSFramework_DIR=${INSTALL_PREFIX}/lib/cmake/DLMSFramework"
    "-DCMAKE_DISABLE_FIND_PACKAGE_OpenSSL=TRUE"
  RESULT_VARIABLE io_configure_result)
if(NOT io_configure_result EQUAL 0)
  message(FATAL_ERROR
    "DLMSFramework install smoke failed during io-only consumer configure")
endif()

execute_process(
  COMMAND "${CMAKE_COMMAND}" --build "${IO_CONSUMER_BUILD_DIR}"
  RESULT_VARIABLE io_build_result)
if(NOT io_build_result EQUAL 0)
  message(FATAL_ERROR
    "DLMSFramework install smoke failed during io-only consumer build")
endif()

foreach(example_name
    codec
    io
    protocol
    cosem_server
    framework
    runtime)
  set(example_source_dir "${SOURCE_DIR}/examples/package-consumers/${example_name}")
  set(example_build_dir "${EXAMPLES_BUILD_DIR}/${example_name}")

  execute_process(
    COMMAND "${CMAKE_COMMAND}" -S "${example_source_dir}" -B "${example_build_dir}"
      -G "${GENERATOR}"
      "-DDLMSFramework_DIR=${INSTALL_PREFIX}/lib/cmake/DLMSFramework"
    RESULT_VARIABLE example_configure_result)
  if(NOT example_configure_result EQUAL 0)
    message(FATAL_ERROR
      "DLMSFramework install smoke failed during ${example_name} example configure")
  endif()

  execute_process(
    COMMAND "${CMAKE_COMMAND}" --build "${example_build_dir}"
    RESULT_VARIABLE example_build_result)
  if(NOT example_build_result EQUAL 0)
    message(FATAL_ERROR
      "DLMSFramework install smoke failed during ${example_name} example build")
  endif()
endforeach()
