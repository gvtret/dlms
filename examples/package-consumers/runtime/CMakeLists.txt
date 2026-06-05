cmake_minimum_required(VERSION 3.16)

project(dlms_runtime_consumer LANGUAGES CXX)

find_package(DLMSFramework REQUIRED CONFIG COMPONENTS runtime)

add_executable(dlms_runtime_consumer main.cpp)
target_compile_features(dlms_runtime_consumer PRIVATE cxx_std_11)
target_link_libraries(dlms_runtime_consumer PRIVATE dlms::runtime)
