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

if(DLMS_WRAPPER_HAS_CODEC_API AND TARGET dlms_wrapper)
  add_executable(dlms_wrapper_integration_tests
    test_wrapper_integration.cpp)

  target_compile_features(dlms_wrapper_integration_tests PRIVATE cxx_std_11)

  target_link_libraries(dlms_wrapper_integration_tests
    PRIVATE
      dlms_wrapper
      GTest::gtest_main)

  gtest_discover_tests(dlms_wrapper_integration_tests)
else()
  message(STATUS "Skipping wrapper integration tests: dlms-wrapper codec API is not implemented yet")
endif()
