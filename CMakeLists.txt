cmake_minimum_required(VERSION 3.16)

project(dlms_integration
  VERSION 0.1.0
  LANGUAGES C CXX)

option(DLMS_BUILD_INTEGRATION_TESTS "Build cross-library integration tests" ON)
option(DLMS_USE_SYSTEM_GTEST "Use system-installed GoogleTest" OFF)
option(DLMS_BUILD_LIVE_TESTS "Build opt-in live meter smoke tests" OFF)

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
