if(DLMS_USE_SYSTEM_GTEST)
  find_package(GTest REQUIRED)
elseif(NOT TARGET GTest::gtest_main)
  include(FetchContent)
  FetchContent_Declare(
    googletest
    URL https://github.com/google/googletest/archive/refs/tags/v1.14.0.zip
    DOWNLOAD_EXTRACT_TIMESTAMP TRUE)
  FetchContent_MakeAvailable(googletest)
endif()

add_executable(dlms_cosem_tests
  cosem/test_cosem_status.cpp
  cosem/test_logical_device.cpp
  cosem/test_object_registry.cpp
  cosem/test_simple_objects.cpp
  cosem/test_cosem_schedule_object.cpp
  cosem/test_cosem_activity_calendar_object.cpp
  cosem/test_cosem_register_monitor_object.cpp
  cosem/test_cosem_demand_register_object.cpp
  cosem/test_cosem_register_activation_object.cpp
  cosem/test_cosem_extended_register_object.cpp
  cosem/test_cosem_register_object.cpp
  cosem/test_cosem_register_table_object.cpp
  cosem/test_cosem_disconnect_control_object.cpp
  cosem/test_cosem_compact_data_object.cpp
  cosem/test_cosem_gprs_modem_setup_object.cpp
  cosem/test_cosem_auto_connect_object.cpp
  cosem/test_cosem_iec_hdlc_setup_object.cpp
  cosem/test_cosem_iso8802_llc_type1_setup_object.cpp
  cosem/test_cosem_iso8802_llc_type2_setup_object.cpp
  cosem/test_cosem_iso8802_llc_type3_setup_object.cpp
  cosem/test_cosem_mac_address_setup_object.cpp
  cosem/test_cosem_parameter_monitor_object.cpp
  cosem/test_cosem_tcp_udp_setup_object.cpp
  cosem/test_cosem_profile_generic_object.cpp
  cosem/test_cosem_script_table_object.cpp
  cosem/test_cosem_sfsk_mac_sync_timeouts_object.cpp
  cosem/types/test_date_time.cpp
  cosem/types/test_date.cpp
  cosem/types/test_time.cpp
  cosem/types/test_script.cpp
  cosem/types/test_single_action_schedule_type.cpp
  cosem/types/test_special_day_entry.cpp
  cosem/types/test_schedule_table_entry.cpp
  cosem/types/test_action_specification.cpp
  cosem/types/test_script_entry.cpp
  cosem/types/test_monitored_value.cpp
  cosem/types/test_action_set.cpp
  cosem/types/test_scaler_unit.cpp
  cosem/types/test_object_definition.cpp
  cosem/types/test_register_mask.cpp)

target_link_libraries(dlms_cosem_tests
  PRIVATE
    dlms_cosem
    GTest::gtest_main)

include(GoogleTest)
gtest_discover_tests(dlms_cosem_tests)
