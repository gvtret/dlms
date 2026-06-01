if(NOT DEFINED VERSION_FILE)
  message(FATAL_ERROR "VERSION_FILE is required")
endif()

if(NOT DEFINED CHANGELOG_FILE)
  message(FATAL_ERROR "CHANGELOG_FILE is required")
endif()

set(DLMS_VERSION_FILE "${VERSION_FILE}")
include("${CMAKE_CURRENT_LIST_DIR}/DlmsVersion.cmake")

if(NOT EXISTS "${CHANGELOG_FILE}")
  message(FATAL_ERROR "Changelog file not found: ${CHANGELOG_FILE}")
endif()

file(READ "${CHANGELOG_FILE}" DLMS_CHANGELOG)
string(REGEX REPLACE "([][+.*^$(){}?|\\\\])" "\\\\\\1" DLMS_VERSION_REGEX "${DLMS_VERSION}")

if(NOT DLMS_CHANGELOG MATCHES "(^|\n)## ${DLMS_VERSION_REGEX}([ \t\r\n-]|$)")
  message(FATAL_ERROR
    "CHANGELOG.md does not contain a release entry for VERSION ${DLMS_VERSION}")
endif()

message(STATUS "DLMS changelog entry found for version: ${DLMS_VERSION}")
