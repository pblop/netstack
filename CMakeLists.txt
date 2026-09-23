cmake_minimum_required(VERSION 3.23)
project(netstack)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_executable(netstack)
target_sources(netstack PRIVATE src/main.cpp src/tap.cpp
               PRIVATE FILE_SET HEADERS BASE_DIRS src FILES src/tap.hpp)

target_compile_options(netstack PRIVATE -Wall -Wextra -fsanitize=address,undefined)
target_link_options(netstack PRIVATE -fsanitize=address,undefined)
