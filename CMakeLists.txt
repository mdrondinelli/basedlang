cmake_minimum_required(VERSION 3.26)
project(basedlang CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_compile_options(-Wall -Wextra -Wpedantic -Werror)

add_subdirectory(extern/Catch2-3.13.0)

add_library(basedlex
  code/basedlex/src/char_stream_reader.cpp
  code/basedlex/src/utf8_char_stream.cpp
)
target_include_directories(basedlex PUBLIC code/basedlex/include)

add_executable(basedc
  code/basedc/src/main.cpp
)
