cmake_minimum_required(VERSION 3.27)
project(KvadraMediaScan)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(kvadra_media_scan src/main.cpp
        include/media_json_builder.h
        src/media_json_builder.cpp
        include/http_server.h
        src/http_server.cpp)


target_include_directories(kvadra_media_scan PRIVATE include)