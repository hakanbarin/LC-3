cmake_minimum_required(VERSION 3.20)

project(LC3_VM VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    add_compile_options(-Wall -Wextra -Wpedantic)
endif()

include_directories(inc)

add_executable(lc3
    src/main.cpp
    src/vm.cpp
    src/terminal.cpp
)

file(COPY .obj/2048.obj DESTINATION ${CMAKE_BINARY_DIR})
file(COPY .obj/rogue.obj DESTINATION ${CMAKE_BINARY_DIR})