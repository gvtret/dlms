cmake_minimum_required(VERSION 3.16)

project(dlms_io_consumer LANGUAGES CXX)

find_package(DLMSFramework REQUIRED CONFIG COMPONENTS io)

add_executable(dlms_io_consumer main.cpp)
target_compile_features(dlms_io_consumer PRIVATE cxx_std_11)
target_link_libraries(dlms_io_consumer PRIVATE dlms::io)
