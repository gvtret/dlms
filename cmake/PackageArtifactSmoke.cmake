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

execute_process(
  COMMAND "${CMAKE_COMMAND}" -E tar tf "${PACKAGE_FILE}"
  RESULT_VARIABLE package_list_result
  OUTPUT_VARIABLE package_entries
  ERROR_VARIABLE package_list_error)
if(NOT package_list_result EQUAL 0)
  message(FATAL_ERROR
    "DLMSFramework package artifact smoke failed while listing package: ${package_list_error}")
endif()

foreach(disallowed_entry
    "/include/gtest/"
    "/include/gmock/"
    "/lib/libgtest"
    "/lib/libgmock")
  if(package_entries MATCHES "(^|\n)[^\n]*${disallowed_entry}")
    message(FATAL_ERROR
      "DLMSFramework package artifact contains test dependency entry matching '${disallowed_entry}'")
  endif()
endforeach()

foreach(required_entry
    "/share/doc/DLMSFramework/README.md"
    "/share/doc/DLMSFramework/CHANGELOG.md"
    "/share/doc/DLMSFramework/VERSION")
  if(NOT package_entries MATCHES "(^|\n)[^\n]*${required_entry}($|\n)")
    message(FATAL_ERROR
      "DLMSFramework package artifact is missing required metadata entry '${required_entry}'")
  endif()
endforeach()

message(STATUS "DLMSFramework package artifact created: ${PACKAGE_FILE}")
