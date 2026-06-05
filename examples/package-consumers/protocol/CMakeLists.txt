cmake_minimum_required(VERSION 3.16)

project(dlms_protocol_consumer LANGUAGES CXX)

find_package(DLMSFramework REQUIRED CONFIG COMPONENTS protocol)

add_executable(dlms_protocol_consumer main.cpp)
target_compile_features(dlms_protocol_consumer PRIVATE cxx_std_11)
target_link_libraries(dlms_protocol_consumer PRIVATE dlms::protocol)
