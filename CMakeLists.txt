cmake_minimum_required(VERSION 3.17)
project(Engine)

set(CMAKE_CXX_STANDARD 20)

add_executable(Engine main.cpp array_wrapper.h window.h camera.h transform.h duplex.h ray.h)

set(BUILD_SHARED_LIBS OFF CACHE BOOL "" FORCE)

add_subdirectory(extern)

target_link_libraries(Engine PRIVATE glm)
target_link_libraries(Engine PRIVATE minifb)