cmake_minimum_required(VERSION 3.16)

project(dlms_integration
  VERSION 0.1.0
  LANGUAGES C CXX)

option(DLMS_BUILD_INTEGRATION_TESTS "Build cross-library integration tests" ON)
option(DLMS_USE_SYSTEM_GTEST "Use system-installed GoogleTest" OFF)
option(DLMS_BUILD_LIVE_TESTS "Build opt-in live meter smoke tests" OFF)
option(DLMS_BUILD_EXAMPLES "Build example programs" ON)

if(DLMS_BUILD_INTEGRATION_TESTS)
  enable_testing()

  if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/lib/dlms-hdlc/CMakeLists.txt" AND
     EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/lib/dlms-llc/CMakeLists.txt")
    add_subdirectory(lib/dlms-hdlc)
    add_subdirectory(lib/dlms-llc)

    if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/lib/dlms-transport/CMakeLists.txt")
      add_subdirectory(lib/dlms-transport)
    endif()

    if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/lib/dlms-apdu/CMakeLists.txt")
      add_subdirectory(lib/dlms-apdu)
    endif()

    if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/lib/dlms-wrapper/CMakeLists.txt")
      add_subdirectory(lib/dlms-wrapper)
    endif()

    if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/lib/dlms-profile/CMakeLists.txt")
      add_subdirectory(lib/dlms-profile)
    endif()

    if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/lib/dlms-association/CMakeLists.txt")
      add_subdirectory(lib/dlms-association)
    endif()

    if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/lib/dlms-security/CMakeLists.txt")
      add_subdirectory(lib/dlms-security)
    endif()

    if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/lib/dlms-cosem/CMakeLists.txt")
      add_subdirectory(lib/dlms-cosem)
    endif()

    if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/lib/dlms-xdlms/CMakeLists.txt")
      add_subdirectory(lib/dlms-xdlms)
    endif()

    if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/lib/dlms-server/CMakeLists.txt")
      add_subdirectory(lib/dlms-server)
    endif()

    if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/lib/dlms-client/CMakeLists.txt")
      add_subdirectory(lib/dlms-client)
    endif()

    if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/lib/dlms-endpoint/CMakeLists.txt")
      add_subdirectory(lib/dlms-endpoint)
    endif()

    if(DLMS_BUILD_EXAMPLES AND TARGET dlms_endpoint)
      add_executable(dlms_endpoint_client_example
        tools/endpoint_client_example.cpp)
      target_compile_features(dlms_endpoint_client_example PRIVATE cxx_std_11)
      target_link_libraries(dlms_endpoint_client_example
        PRIVATE
          dlms_endpoint)

      add_executable(dlms_endpoint_server_example
        tools/endpoint_server_example.cpp)
      target_compile_features(dlms_endpoint_server_example PRIVATE cxx_std_11)
      target_link_libraries(dlms_endpoint_server_example
        PRIVATE
          dlms_endpoint)

      add_executable(dlms_endpoint_custom_server_example
        tools/endpoint_custom_server_example.cpp)
      target_compile_features(dlms_endpoint_custom_server_example PRIVATE cxx_std_11)
      target_link_libraries(dlms_endpoint_custom_server_example
        PRIVATE
          dlms_endpoint)

      add_executable(dlms_endpoint_custom_server_listener_example
        tools/endpoint_custom_server_listener_example.cpp)
      target_compile_features(dlms_endpoint_custom_server_listener_example PRIVATE cxx_std_11)
      target_link_libraries(dlms_endpoint_custom_server_listener_example
        PRIVATE
          dlms_endpoint)

      add_executable(dlms_client_custom_xdlms_service_example
        tools/client_custom_xdlms_service_example.cpp)
      target_compile_features(dlms_client_custom_xdlms_service_example PRIVATE cxx_std_11)
      target_link_libraries(dlms_client_custom_xdlms_service_example
        PRIVATE
          dlms_client)

      add_executable(dlms_endpoint_push_listener_example
        tools/endpoint_push_listener_example.cpp)
      target_compile_features(dlms_endpoint_push_listener_example PRIVATE cxx_std_11)
      target_link_libraries(dlms_endpoint_push_listener_example
        PRIVATE
          dlms_endpoint)

      add_executable(dlms_endpoint_gateway_example
        tools/endpoint_gateway_example.cpp)
      target_compile_features(dlms_endpoint_gateway_example PRIVATE cxx_std_11)
      target_link_libraries(dlms_endpoint_gateway_example
        PRIVATE
          dlms_endpoint)

      add_test(NAME dlms_endpoint_client_example
        COMMAND dlms_endpoint_client_example)
      add_test(NAME dlms_endpoint_server_example
        COMMAND dlms_endpoint_server_example)
      add_test(NAME dlms_endpoint_custom_server_example
        COMMAND dlms_endpoint_custom_server_example)
      add_test(NAME dlms_endpoint_custom_server_listener_example
        COMMAND dlms_endpoint_custom_server_listener_example)
      add_test(NAME dlms_client_custom_xdlms_service_example
        COMMAND dlms_client_custom_xdlms_service_example)
      add_test(NAME dlms_endpoint_push_listener_example
        COMMAND dlms_endpoint_push_listener_example)
      add_test(NAME dlms_endpoint_gateway_example
        COMMAND dlms_endpoint_gateway_example)
    endif()

    if(DLMS_BUILD_LIVE_TESTS AND TARGET dlms_client)
      add_executable(dlms_live_meter_smoke
        tools/live_meter_smoke.cpp)

      target_compile_features(dlms_live_meter_smoke PRIVATE cxx_std_11)

      target_link_libraries(dlms_live_meter_smoke
        PRIVATE
          dlms_client)

      add_test(NAME LiveMeterSmoke COMMAND dlms_live_meter_smoke)
    endif()

    if(DLMS_LLC_HAS_CODEC_API)
      add_subdirectory(test/integration)
    else()
      message(STATUS "Skipping integration tests: dlms-llc codec API is not implemented yet")
    endif()
  else()
    message(STATUS "Skipping integration tests: dlms-hdlc or dlms-llc CMakeLists.txt is missing")
  endif()
endif()
