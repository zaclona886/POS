cmake_minimum_required(VERSION 3.18)
project(klient C)

set(CMAKE_C_STANDARD 99)

add_executable(klient main.c)

target_link_libraries(klient pthread)
