cmake_minimum_required(VERSION 3.10)

project(TransportCatalog LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(TransportCatalog
    sourse/main.cpp
    sourse/domain.cpp
    sourse/geo.cpp
    core/transport_catalogue.cpp
    core/transport_router.cpp
    outs/map_renderer.cpp
    outs/stat_reader.cpp
    outs/svg.cpp
    request/request_handler.cpp
    inputs/json.cpp
    inputs/input_reader.cpp
    inputs/json_builder.cpp
    inputs/json_reader.cpp
)

target_include_directories(TransportCatalog PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/sourse
    ${CMAKE_CURRENT_SOURCE_DIR}/core
    ${CMAKE_CURRENT_SOURCE_DIR}/outs
    ${CMAKE_CURRENT_SOURCE_DIR}/inputs
    ${CMAKE_CURRENT_SOURCE_DIR}/request
)
