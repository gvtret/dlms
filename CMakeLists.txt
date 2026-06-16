cmake_minimum_required(VERSION 3.16)

include(cmake/DlmsVersion.cmake)

project(dlms_integration
  VERSION ${DLMS_PROJECT_VERSION}
  LANGUAGES C CXX)

include(GNUInstallDirs)
include(CMakePackageConfigHelpers)

option(DLMS_BUILD_INTEGRATION_TESTS "Build cross-library integration tests" ON)
option(DLMS_USE_SYSTEM_GTEST "Use system-installed GoogleTest" OFF)
option(DLMS_BUILD_LIVE_TESTS "Build opt-in live meter smoke tests" OFF)
option(DLMS_BUILD_EXAMPLES "Build example programs" ON)
option(DLMS_INSTALL "Install the DLMS framework CMake package" ON)

if(DLMS_BUILD_INTEGRATION_TESTS AND NOT DLMS_USE_SYSTEM_GTEST)
  set(INSTALL_GTEST OFF CACHE BOOL "Install GoogleTest with the framework package" FORCE)
endif()

if(DLMS_BUILD_INTEGRATION_TESTS)
  enable_testing()

  add_test(NAME dlms_semver_check
    COMMAND ${CMAKE_COMMAND}
      -DVERSION_FILE=${CMAKE_CURRENT_SOURCE_DIR}/VERSION
      -P ${CMAKE_CURRENT_SOURCE_DIR}/cmake/CheckSemver.cmake)

  add_test(NAME dlms_changelog_check
    COMMAND ${CMAKE_COMMAND}
      -DVERSION_FILE=${CMAKE_CURRENT_SOURCE_DIR}/VERSION
      -DCHANGELOG_FILE=${CMAKE_CURRENT_SOURCE_DIR}/CHANGELOG.md
      -P ${CMAKE_CURRENT_SOURCE_DIR}/cmake/CheckChangelog.cmake)

  add_test(NAME dlms_release_tag_check
    COMMAND ${CMAKE_COMMAND}
      -DVERSION_FILE=${CMAKE_CURRENT_SOURCE_DIR}/VERSION
      -P ${CMAKE_CURRENT_SOURCE_DIR}/cmake/CheckReleaseTag.cmake)

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

    include(cmake/DlmsPackages.cmake)

    if(DLMS_INSTALL AND TARGET dlms_framework)
      set(DLMS_COMPONENT_TARGETS
        dlms_hdlc
        dlms_llc
        dlms_wrapper
        dlms_apdu
        dlms_transport
        dlms_profile
        dlms_association
        dlms_security
        dlms_xdlms
        dlms_cosem
        dlms_server
        dlms_client
        dlms_endpoint)

      set(DLMS_PACKAGE_TARGETS
        dlms_codec
        dlms_io
        dlms_protocol
        dlms_cosem_server
        dlms_runtime
        dlms_framework)

      install(TARGETS ${DLMS_COMPONENT_TARGETS} ${DLMS_PACKAGE_TARGETS}
        EXPORT DLMSFrameworkTargets
        ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})

      foreach(dlms_include_dir
          lib/dlms-hdlc/include
          lib/dlms-llc/include
          lib/dlms-wrapper/include
          lib/dlms-apdu/include
          lib/dlms-transport/include
          lib/dlms-profile/include
          lib/dlms-association/include
          lib/dlms-security/include
          lib/dlms-xdlms/include
          lib/dlms-cosem/include
          lib/dlms-server/include
          lib/dlms-client/include
          lib/dlms-endpoint/include)
        install(DIRECTORY ${dlms_include_dir}/
          DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})
      endforeach()

      configure_package_config_file(
        cmake/DLMSFrameworkConfig.cmake.in
        ${CMAKE_CURRENT_BINARY_DIR}/DLMSFrameworkConfig.cmake
        INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/DLMSFramework)

      write_basic_package_version_file(
        ${CMAKE_CURRENT_BINARY_DIR}/DLMSFrameworkConfigVersion.cmake
        VERSION ${PROJECT_VERSION}
        COMPATIBILITY SameMajorVersion)

      install(FILES
        ${CMAKE_CURRENT_BINARY_DIR}/DLMSFrameworkConfig.cmake
        ${CMAKE_CURRENT_BINARY_DIR}/DLMSFrameworkConfigVersion.cmake
        DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/DLMSFramework)

      install(FILES
        README.md
        CHANGELOG.md
        VERSION
        DESTINATION ${CMAKE_INSTALL_DATADIR}/doc/DLMSFramework)

      install(EXPORT DLMSFrameworkTargets
        NAMESPACE dlms::
        DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/DLMSFramework)

      add_test(NAME dlms_package_install_smoke
        COMMAND ${CMAKE_COMMAND}
          -DBINARY_DIR=${CMAKE_CURRENT_BINARY_DIR}
          -DSOURCE_DIR=${CMAKE_CURRENT_SOURCE_DIR}
          -DGENERATOR=${CMAKE_GENERATOR}
          -P ${CMAKE_CURRENT_SOURCE_DIR}/cmake/PackageInstallSmoke.cmake)

      set(CPACK_PACKAGE_NAME "DLMSFramework")
      set(CPACK_PACKAGE_VENDOR "DLMS Framework")
      set(CPACK_PACKAGE_CONTACT "DLMS Framework maintainers")
      set(CPACK_PACKAGE_VERSION "${DLMS_VERSION}")
      set(CPACK_PACKAGE_FILE_NAME "DLMSFramework-${DLMS_VERSION}")
      set(CPACK_GENERATOR "ZIP")
      include(CPack)

      add_test(NAME dlms_package_artifact_smoke
        COMMAND ${CMAKE_COMMAND}
          -DBINARY_DIR=${CMAKE_CURRENT_BINARY_DIR}
          -DPACKAGE_FILE=${CMAKE_CURRENT_BINARY_DIR}/DLMSFramework-${DLMS_VERSION}.zip
          -P ${CMAKE_CURRENT_SOURCE_DIR}/cmake/PackageArtifactSmoke.cmake)
    endif()

    if(TARGET dlms_framework)
      add_executable(dlms_framework_target_smoke
        tools/framework_target_smoke.cpp)
      target_compile_features(dlms_framework_target_smoke PRIVATE cxx_std_11)
      target_link_libraries(dlms_framework_target_smoke
        PRIVATE
          dlms_framework)

      add_test(NAME dlms_framework_target_smoke
        COMMAND dlms_framework_target_smoke)
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

      add_executable(dlms_client_gui_read_example
        tools/client_gui_read_example.cpp)
      target_compile_features(dlms_client_gui_read_example PRIVATE cxx_std_11)
      target_link_libraries(dlms_client_gui_read_example
        PRIVATE
          dlms_client)

      add_executable(dlms_client_spodes_obis_example
        tools/client_spodes_obis_example.cpp)
      target_compile_features(dlms_client_spodes_obis_example PRIVATE cxx_std_11)
      target_link_libraries(dlms_client_spodes_obis_example
        PRIVATE
          dlms_client
          dlms_cosem)

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
      add_test(NAME dlms_client_gui_read_example
        COMMAND dlms_client_gui_read_example)
      add_test(NAME dlms_client_spodes_obis_example
        COMMAND dlms_client_spodes_obis_example)
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

      # Redaction policy regression test for the live smoke
      # tool. Pins P0 §3.6 trace-secrets behaviour without
      # needing a real meter.
      add_executable(dlms_live_meter_smoke_redaction_tests
        tools/test/test_live_meter_smoke_redaction.cpp)

      target_compile_features(dlms_live_meter_smoke_redaction_tests
        PRIVATE cxx_std_11)

      target_include_directories(dlms_live_meter_smoke_redaction_tests
        PRIVATE
          ${CMAKE_CURRENT_SOURCE_DIR}/tools)

      target_link_libraries(dlms_live_meter_smoke_redaction_tests
        PRIVATE
          dlms_profile
          GTest::gtest_main)

      include(GoogleTest)
      if(WIN32)
        get_filename_component(DLMS_MINGW_BIN_DIR
          "${CMAKE_CXX_COMPILER}" DIRECTORY)
        add_test(NAME dlms_live_meter_smoke_redaction_tests
          COMMAND ${CMAKE_COMMAND} -E env
            "PATH=${DLMS_MINGW_BIN_DIR}"
            $<TARGET_FILE:dlms_live_meter_smoke_redaction_tests>)
      else()
        gtest_discover_tests(dlms_live_meter_smoke_redaction_tests)
      endif()
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
