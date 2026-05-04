add_executable(dlms_llc_hdlc_integration_tests
  test_llc_hdlc_integration.cpp)

target_compile_features(dlms_llc_hdlc_integration_tests PRIVATE cxx_std_11)

target_link_libraries(dlms_llc_hdlc_integration_tests
  PRIVATE
    dlms_hdlc
    dlms_llc
    GTest::gtest_main)

include(GoogleTest)
gtest_discover_tests(dlms_llc_hdlc_integration_tests)
