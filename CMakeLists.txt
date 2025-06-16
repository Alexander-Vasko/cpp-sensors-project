cmake_minimum_required(VERSION 3.15)
project(cpp_sensors_project LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_subdirectory(contribs/clickhouse-cpp)

if(POLICY CMP0167)
    cmake_policy(SET CMP0167 NEW)
endif()

find_package(Boost 1.74.0 REQUIRED COMPONENTS system thread)
find_package(nlohmann_json REQUIRED)

# Пути к исходникам
set(SRC_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src)

file(GLOB_RECURSE SRC_SOURCES
    ${SRC_DIR}/*.cpp
)

# Добавляем main.cpp из корня проекта
set(SOURCES
    ${SRC_SOURCES}
    ${CMAKE_CURRENT_SOURCE_DIR}/main.cpp
)

add_executable(cpp_sensors_project ${SOURCES})

target_include_directories(cpp_sensors_project PRIVATE
    ${SRC_DIR}
)

target_link_libraries(cpp_sensors_project PRIVATE
    clickhouse-cpp-lib
    Boost::system
    Boost::thread
    nlohmann_json::nlohmann_json
)

