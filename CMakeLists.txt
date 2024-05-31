cmake_minimum_required(VERSION 3.22.1)

project(Raycasting2)

find_package(SDL2 REQUIRED)
include_directories(Raycasting2 ${SDL2_INCLUDE_DIRS})

add_executable(Raycasting2 main.cpp)
target_link_libraries(Raycasting2 ${SDL2_LIBRARIES})