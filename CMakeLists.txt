cmake_minimum_required(VERSION 3.16)

project(dlms_integration
  VERSION 0.1.0
  LANGUAGES C CXX)

option(DLMS_BUILD_INTEGRATION_TESTS "Build cross-library integration tests" ON)
option(DLMS_USE_SYSTEM_GTEST "Use system-installed GoogleTest" OFF)

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

    if(DLMS_LLC_HAS_CODEC_API)
      add_subdirectory(test/integration)
    else()
      message(STATUS "Skipping integration tests: dlms-llc codec API is not implemented yet")
    endif()
  else()
    message(STATUS "Skipping integration tests: dlms-hdlc or dlms-llc CMakeLists.txt is missing")
  endif()
endif()
