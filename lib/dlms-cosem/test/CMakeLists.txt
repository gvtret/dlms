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
  cosem/test_logical_device.cpp
  cosem/test_object_registry.cpp
  cosem/test_simple_objects.cpp)

target_link_libraries(dlms_cosem_tests
  PRIVATE
    dlms_cosem
    GTest::gtest_main)

include(GoogleTest)
gtest_discover_tests(dlms_cosem_tests)
