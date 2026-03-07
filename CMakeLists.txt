cmake_minimum_required(VERSION 3.26)
project(basedlang CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_compile_options(-Wall -Wextra -Wpedantic -Werror)

add_subdirectory(extern/Catch2-3.13.0)

add_executable(basedc
  code/basedc/src/main.cpp
)
