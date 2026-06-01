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

find_package(DLMSFramework REQUIRED CONFIG)

add_executable(dlms_package_consumer main.cpp)
target_compile_features(dlms_package_consumer PRIVATE cxx_std_11)
target_link_libraries(dlms_package_consumer PRIVATE dlms::framework)
]=])

file(WRITE "${CONSUMER_DIR}/main.cpp" [=[
#include "dlms/apdu/apdu_types.hpp"
#include "dlms/transport/transport_status.hpp"

#include <cstddef>
#include <cstdint>

int main()
{
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
