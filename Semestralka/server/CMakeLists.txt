cmake_minimum_required(VERSION 3.18)
project(server C)

set(CMAKE_C_STANDARD 99)

add_executable(server main.c)

target_link_libraries(server pthread)
