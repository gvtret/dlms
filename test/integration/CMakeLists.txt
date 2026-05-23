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

if(DLMS_APDU_HAS_CODEC_API AND TARGET dlms_apdu)
  add_executable(dlms_apdu_stack_integration_tests
    test_apdu_stack_integration.cpp)

  target_compile_features(dlms_apdu_stack_integration_tests PRIVATE cxx_std_11)

  target_link_libraries(dlms_apdu_stack_integration_tests
    PRIVATE
      dlms_apdu
      dlms_hdlc
      dlms_llc
      GTest::gtest_main)

  if(DLMS_WRAPPER_HAS_CODEC_API AND TARGET dlms_wrapper)
    target_link_libraries(dlms_apdu_stack_integration_tests
      PRIVATE
        dlms_wrapper)

    target_compile_definitions(dlms_apdu_stack_integration_tests
      PRIVATE
        DLMS_APDU_STACK_HAS_WRAPPER=1)
  endif()

  gtest_discover_tests(dlms_apdu_stack_integration_tests)
else()
  message(STATUS "Skipping APDU stack integration tests: dlms-apdu codec API is not implemented yet")
endif()
if(DLMS_TRANSPORT_HAS_API AND TARGET dlms_transport)
  add_executable(dlms_transport_profile_integration_tests
    test_transport_profile_integration.cpp)

  target_compile_features(dlms_transport_profile_integration_tests PRIVATE cxx_std_11)

  target_link_libraries(dlms_transport_profile_integration_tests
    PRIVATE
      dlms_transport
      GTest::gtest_main)

  gtest_discover_tests(dlms_transport_profile_integration_tests)
else()
  message(STATUS "Skipping transport profile integration tests: dlms-transport API is not available")
endif()

if(DLMS_PROFILE_HAS_API AND TARGET dlms_profile AND
   DLMS_APDU_HAS_CODEC_API AND TARGET dlms_apdu)
  add_executable(dlms_profile_integration_tests
    test_profile_integration.cpp)

  target_compile_features(dlms_profile_integration_tests PRIVATE cxx_std_11)

  target_link_libraries(dlms_profile_integration_tests
    PRIVATE
      dlms_profile
      dlms_apdu
      GTest::gtest_main)

  if(WIN32)
    get_filename_component(DLMS_MINGW_BIN_DIR
      "${CMAKE_CXX_COMPILER}" DIRECTORY)
    add_test(NAME dlms_profile_integration_tests
      COMMAND ${CMAKE_COMMAND} -E env
        "PATH=${DLMS_MINGW_BIN_DIR}"
        $<TARGET_FILE:dlms_profile_integration_tests>)
  else()
    gtest_discover_tests(dlms_profile_integration_tests)
  endif()
else()
  message(STATUS "Skipping profile integration tests: dlms-profile or dlms-apdu API is not available")
endif()

if(DLMS_SECURITY_HAS_API AND TARGET dlms_security)
  add_executable(dlms_security_integration_tests
    test_security_integration.cpp)

  target_compile_features(dlms_security_integration_tests PRIVATE cxx_std_11)

  target_link_libraries(dlms_security_integration_tests
    PRIVATE
      dlms_security
      GTest::gtest_main)

  gtest_discover_tests(dlms_security_integration_tests)
else()
  message(STATUS "Skipping security integration tests: dlms-security API is not available")
endif()

if(TARGET dlms_security AND TARGET dlms_xdlms AND TARGET dlms_server AND
   TARGET dlms_cosem AND TARGET dlms_association AND TARGET dlms_profile AND
   TARGET dlms_apdu)
  add_executable(dlms_ciphered_get_integration_tests
    test_ciphered_get_integration.cpp)

  target_compile_features(
    dlms_ciphered_get_integration_tests PRIVATE cxx_std_11)

  target_link_libraries(dlms_ciphered_get_integration_tests
    PRIVATE
      dlms_security
      dlms_xdlms
      dlms_server
      dlms_cosem
      dlms_association
      dlms_profile
      dlms_apdu
      GTest::gtest_main)

  gtest_discover_tests(dlms_ciphered_get_integration_tests)
else()
  message(STATUS "Skipping ciphered GET integration tests: protected stack is not available")
endif()

if(TARGET dlms_xdlms AND TARGET dlms_association AND
   TARGET dlms_profile AND TARGET dlms_apdu)
  add_executable(dlms_xdlms_integration_tests
    test_xdlms_integration.cpp)

  target_compile_features(dlms_xdlms_integration_tests PRIVATE cxx_std_11)

  target_link_libraries(dlms_xdlms_integration_tests
    PRIVATE
      dlms_xdlms
      dlms_association
      dlms_profile
      dlms_apdu
      GTest::gtest_main)

  gtest_discover_tests(dlms_xdlms_integration_tests)
else()
  message(STATUS "Skipping xDLMS integration tests: dlms-xdlms stack is not available")
endif()

if(TARGET dlms_xdlms AND TARGET dlms_server AND
   TARGET dlms_cosem AND TARGET dlms_apdu)
  add_executable(dlms_server_get_apdu_integration_tests
    test_server_get_apdu_integration.cpp)

  target_compile_features(dlms_server_get_apdu_integration_tests PRIVATE cxx_std_11)

  target_link_libraries(dlms_server_get_apdu_integration_tests
    PRIVATE
      dlms_xdlms
      dlms_server
      dlms_cosem
      dlms_apdu
      GTest::gtest_main)

  gtest_discover_tests(dlms_server_get_apdu_integration_tests)
else()
  message(STATUS "Skipping server GET APDU integration tests: server xDLMS stack is not available")
endif()

if(TARGET dlms_client AND TARGET dlms_xdlms AND TARGET dlms_server AND
   TARGET dlms_cosem AND TARGET dlms_association AND TARGET dlms_profile AND
   TARGET dlms_apdu)
  add_executable(dlms_client_service_integration_tests
    test_client_service_integration.cpp)

  target_compile_features(
    dlms_client_service_integration_tests PRIVATE cxx_std_11)

  target_link_libraries(dlms_client_service_integration_tests
    PRIVATE
      dlms_client
      dlms_xdlms
      dlms_server
      dlms_cosem
      dlms_association
      dlms_profile
      dlms_apdu
      GTest::gtest_main)

  if(WIN32)
    get_filename_component(DLMS_MINGW_BIN_DIR
      "${CMAKE_CXX_COMPILER}" DIRECTORY)
    add_test(NAME dlms_client_service_integration_tests
      COMMAND ${CMAKE_COMMAND} -E env
        "PATH=${DLMS_MINGW_BIN_DIR}"
        $<TARGET_FILE:dlms_client_service_integration_tests>)
  else()
    gtest_discover_tests(dlms_client_service_integration_tests)
  endif()
else()
  message(STATUS "Skipping client service integration tests: client/server stack is not available")
endif()

if(TARGET dlms_endpoint AND TARGET dlms_apdu AND TARGET dlms_cosem)
  add_executable(dlms_endpoint_integration_tests
    test_endpoint_integration.cpp)

  target_compile_features(
    dlms_endpoint_integration_tests PRIVATE cxx_std_11)

  target_link_libraries(dlms_endpoint_integration_tests
    PRIVATE
      dlms_endpoint
      dlms_apdu
      dlms_cosem
      GTest::gtest_main)

  if(WIN32)
    get_filename_component(DLMS_MINGW_BIN_DIR
      "${CMAKE_CXX_COMPILER}" DIRECTORY)
    add_test(NAME dlms_endpoint_integration_tests
      COMMAND ${CMAKE_COMMAND} -E env
        "PATH=${DLMS_MINGW_BIN_DIR}"
        $<TARGET_FILE:dlms_endpoint_integration_tests>)
  else()
    gtest_discover_tests(dlms_endpoint_integration_tests)
  endif()
else()
  message(STATUS "Skipping endpoint integration tests: dlms-endpoint stack is not available")
endif()
