if(NOT DEFINED VERSION_FILE)
  message(FATAL_ERROR "VERSION_FILE is required")
endif()

set(DLMS_VERSION_FILE "${VERSION_FILE}")
include("${CMAKE_CURRENT_LIST_DIR}/DlmsVersion.cmake")

message(STATUS "DLMS semantic version: ${DLMS_VERSION}")
