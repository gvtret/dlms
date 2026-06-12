cmake_minimum_required(VERSION 3.16)

project(dlms_framework_consumer LANGUAGES CXX)

find_package(DLMSFramework REQUIRED CONFIG COMPONENTS framework)

add_executable(dlms_framework_consumer main.cpp)
target_compile_features(dlms_framework_consumer PRIVATE cxx_std_11)
target_link_libraries(dlms_framework_consumer PRIVATE dlms::framework)
