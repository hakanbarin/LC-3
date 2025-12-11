cmake_minimum_required(VERSION 3.20)

project(LC3_VM VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)


include_directories(inc)

add_executable(lc3
    src/main.cpp
)

file(COPY .obj/2048.obj  DESTINATION ${CMAKE_BINARY_DIR})
file(COPY .obj/rogue.obj  DESTINATION ${CMAKE_BINARY_DIR})