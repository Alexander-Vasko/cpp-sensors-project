cmake_minimum_required(VERSION 3.12)
project(cpp_sencors_project)

set(CMAKE_CXX_STANDARD 20)

add_subdirectory(contribs/clickhouse-cpp)

find_package (Boost 1.74.0 REQUIRED COMPONENTS thread)
find_package (nlohmann_json REQUIRED)

add_executable(cpp_sencors_project main.cpp)

target_link_libraries(cpp_sencors_project PRIVATE
    clickhouse-cpp-lib
    Boost::thread
    nlohmann_json::nlohmann_json
)
