cmake_minimum_required(VERSION 3.20)
project(surge VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_compile_options(-Wall -Wextra -Wpedantic -Werror)

add_executable(surge
    src/main.cpp
    src/cli/parser.cpp
)

target_include_directories(surge PRIVATE ${CMAKE_SOURCE_DIR}/src)