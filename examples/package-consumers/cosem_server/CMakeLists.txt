cmake_minimum_required(VERSION 3.16)

project(dlms_cosem_server_consumer LANGUAGES CXX)

find_package(DLMSFramework REQUIRED CONFIG COMPONENTS cosem_server)

add_executable(dlms_cosem_server_consumer main.cpp)
target_compile_features(dlms_cosem_server_consumer PRIVATE cxx_std_11)
target_link_libraries(dlms_cosem_server_consumer PRIVATE dlms::cosem_server)
