cmake_minimum_required(VERSION 3.22)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED true)

project(yjcServer)


find_package(spdlog REQUIRED PATHS "${CMAKE_SOURCE_DIR}/yjcServer/spdlog/build")

add_subdirectory(yjcServer)
add_subdirectory(tests)