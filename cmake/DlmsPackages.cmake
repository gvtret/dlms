function(dlms_add_interface_package package_name)
  set(package_targets ${ARGN})

  foreach(package_target IN LISTS package_targets)
    if(NOT TARGET ${package_target})
      message(FATAL_ERROR
        "Cannot create ${package_name}: required target ${package_target} is missing")
    endif()
  endforeach()

  if(NOT TARGET ${package_name})
    add_library(${package_name} INTERFACE)
    target_link_libraries(${package_name}
      INTERFACE
        ${package_targets})
  endif()
endfunction()

dlms_add_interface_package(dlms_codec
  dlms_hdlc
  dlms_llc
  dlms_wrapper
  dlms_apdu)

dlms_add_interface_package(dlms_io
  dlms_transport
  dlms_profile)

dlms_add_interface_package(dlms_protocol
  dlms_association
  dlms_security
  dlms_xdlms)

dlms_add_interface_package(dlms_cosem_server
  dlms_cosem
  dlms_server)

dlms_add_interface_package(dlms_runtime
  dlms_client
  dlms_endpoint)

dlms_add_interface_package(dlms_framework
  dlms_codec
  dlms_io
  dlms_protocol
  dlms_cosem_server
  dlms_runtime)

add_library(dlms::codec ALIAS dlms_codec)
add_library(dlms::io ALIAS dlms_io)
add_library(dlms::protocol ALIAS dlms_protocol)
add_library(dlms::cosem_server ALIAS dlms_cosem_server)
add_library(dlms::runtime ALIAS dlms_runtime)
add_library(dlms::framework ALIAS dlms_framework)
