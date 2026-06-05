if(NOT DEFINED BINARY_DIR)
  message(FATAL_ERROR "BINARY_DIR is required")
endif()

if(NOT DEFINED GENERATOR)
  message(FATAL_ERROR "GENERATOR is required")
endif()

set(SMOKE_DIR "${BINARY_DIR}/package-smoke")
set(INSTALL_PREFIX "${SMOKE_DIR}/install")
set(CONSUMER_DIR "${SMOKE_DIR}/consumer")
set(CONSUMER_BUILD_DIR "${SMOKE_DIR}/consumer-build")

file(REMOVE_RECURSE "${SMOKE_DIR}")
file(MAKE_DIRECTORY "${CONSUMER_DIR}")

execute_process(
  COMMAND "${CMAKE_COMMAND}" --install "${BINARY_DIR}" --prefix "${INSTALL_PREFIX}"
  RESULT_VARIABLE install_result)
if(NOT install_result EQUAL 0)
  message(FATAL_ERROR "DLMSFramework install smoke failed during install")
endif()

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
