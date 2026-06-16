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

add_executable(dlms_apdu_tests
  apdu/test_acse.cpp
  apdu/test_apdu_c_api.cpp
  apdu/test_apdu_c_header.c
  apdu/test_action.cpp
  apdu/test_apdu_error.cpp
  apdu/test_apdu_status.cpp
  apdu/test_ber.cpp
  apdu/test_axdr.cpp
  apdu/test_data.cpp
  apdu/test_get.cpp
  apdu/test_initiate.cpp
  apdu/test_set.cpp
  apdu/test_xdlms.cpp
  apdu/test_apdu_reader.cpp
  apdu/test_apdu_writer.cpp)

target_link_libraries(dlms_apdu_tests
  PRIVATE
    dlms_apdu
    GTest::gtest_main)

include(GoogleTest)
gtest_discover_tests(dlms_apdu_tests)

add_executable(dlms_apdu_c_header_smoke
  apdu/test_apdu_c_header.c
  apdu/test_apdu_c_header_main.c)

target_link_libraries(dlms_apdu_c_header_smoke
  PRIVATE
    dlms_apdu)

add_test(
  NAME ApduCApi.CHeaderCompilesAsC
  COMMAND dlms_apdu_c_header_smoke)
