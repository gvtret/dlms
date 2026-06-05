cmake_minimum_required(VERSION 3.16)

project(dlms_codec_consumer LANGUAGES CXX)

find_package(DLMSFramework REQUIRED CONFIG COMPONENTS codec)

add_executable(dlms_codec_consumer main.cpp)
target_compile_features(dlms_codec_consumer PRIVATE cxx_std_11)
target_link_libraries(dlms_codec_consumer PRIVATE dlms::codec)
