cmake_minimum_required(VERSION 3.0.2...3.32.0)
project(cpp_sencors_project)

set(CMAKE_CXX_STANDARD 20)

add_subdirectory(contribs/clickhouse-cpp)

if(POLICY CMP0167)
    cmake_policy(SET CMP0167 NEW)
endif()

find_package (Boost 1.74.0 REQUIRED COMPONENTS thread)
find_package (nlohmann_json REQUIRED)

add_executable(cpp_sencors_project main.cpp)

target_link_libraries(cpp_sencors_project PRIVATE
    clickhouse-cpp-lib
    Boost::thread
    nlohmann_json::nlohmann_json
)
