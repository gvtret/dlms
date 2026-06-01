if(NOT DEFINED BINARY_DIR)
  message(FATAL_ERROR "BINARY_DIR is required")
endif()

if(NOT DEFINED PACKAGE_FILE)
  message(FATAL_ERROR "PACKAGE_FILE is required")
endif()

execute_process(
  COMMAND "${CMAKE_COMMAND}" --build "${BINARY_DIR}" --target package
  RESULT_VARIABLE package_result)
if(NOT package_result EQUAL 0)
  message(FATAL_ERROR "DLMSFramework package artifact smoke failed during package build")
endif()

if(NOT EXISTS "${PACKAGE_FILE}")
  message(FATAL_ERROR "DLMSFramework package artifact was not created: ${PACKAGE_FILE}")
endif()

file(SIZE "${PACKAGE_FILE}" package_size)
if(package_size EQUAL 0)
  message(FATAL_ERROR "DLMSFramework package artifact is empty: ${PACKAGE_FILE}")
endif()

message(STATUS "DLMSFramework package artifact created: ${PACKAGE_FILE}")
