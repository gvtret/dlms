if(DLMS_USE_SYSTEM_GTEST)
  find_package(GTest REQUIRED)
elseif(NOT TARGET GTest::gtest_main)
  include(FetchContent)
  FetchContent_Declare(
    googletest
    URL https://github.com/google/googletest/archive/refs/tags/v1.14.0.zip
    DOWNLOAD_EXTRACT_TIMESTAMP TRUE)
  FetchContent_MakeAvailable(googletest)
endif()

add_executable(dlms_endpoint_tests
  endpoint/test_client_endpoint.cpp
  endpoint/test_endpoint_factories.cpp
  endpoint/test_gateway_endpoint.cpp
  endpoint/test_listener_runtime.cpp
  endpoint/test_endpoint_options.cpp
  endpoint/test_push_listener_endpoint.cpp
  endpoint/test_server_endpoint.cpp
  endpoint/test_endpoint_status.cpp)

target_link_libraries(dlms_endpoint_tests
  PRIVATE
    dlms_endpoint
    GTest::gtest_main)

include(GoogleTest)
if(WIN32)
  get_filename_component(DLMS_MINGW_BIN_DIR
    "${CMAKE_CXX_COMPILER}" DIRECTORY)
  add_test(NAME dlms_endpoint_tests
    COMMAND ${CMAKE_COMMAND} -E env
      "PATH=${DLMS_MINGW_BIN_DIR}"
      $<TARGET_FILE:dlms_endpoint_tests>)
else()
  gtest_discover_tests(dlms_endpoint_tests)
endif()
