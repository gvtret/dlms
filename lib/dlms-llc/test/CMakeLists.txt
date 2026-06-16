if(DLMS_USE_SYSTEM_GTEST)
  find_package(GTest REQUIRED)
else()
  include(FetchContent)
  FetchContent_Declare(
    googletest
    URL https://github.com/google/googletest/archive/refs/tags/v1.14.0.zip
    DOWNLOAD_EXTRACT_TIMESTAMP TRUE)
  FetchContent_MakeAvailable(googletest)
endif()

add_executable(dlms_llc_tests
  llc/test_llc_c_api.cpp
  llc/test_llc_c_header.c
  llc/test_llc_codec.cpp
  llc/test_llc_header.cpp
  llc/test_llc_vectors.cpp)

target_link_libraries(dlms_llc_tests
  PRIVATE
    dlms_llc
    GTest::gtest_main)

include(GoogleTest)
gtest_discover_tests(dlms_llc_tests)

add_executable(dlms_llc_c_header_smoke
  llc/test_llc_c_header.c
  llc/test_llc_c_header_main.c)

target_link_libraries(dlms_llc_c_header_smoke
  PRIVATE
    dlms_llc)

add_test(
  NAME LlcCApi.CHeaderCompilesAsC
  COMMAND dlms_llc_c_header_smoke)
