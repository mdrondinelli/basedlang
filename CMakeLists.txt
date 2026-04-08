cmake_minimum_required(VERSION 3.22)
project(kalelang CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_compile_options(-Wall -Wextra -Wpedantic -Werror)

add_subdirectory(extern/Catch2-3.13.0)

add_library(kalelex
  code/kalelex/src/char_stream_reader.cpp
  code/kalelex/src/istream_binary_stream.cpp
  code/kalelex/src/lexeme_stream.cpp
  code/kalelex/src/lexeme_stream_reader.cpp
  code/kalelex/src/utf8_char_stream.cpp
)
target_include_directories(kalelex PUBLIC code/kalelex/include)

add_library(kaleast
  code/kaleast/src/ast.cpp
  code/kaleast/src/operator.cpp
  code/kaleast/src/source_span.cpp
)
target_include_directories(kaleast PUBLIC code/kaleast/include)
target_link_libraries(kaleast PUBLIC kalelex)

add_library(kaleparse
  code/kaleparse/src/parser.cpp
)
target_include_directories(kaleparse PUBLIC code/kaleparse/include)
target_link_libraries(kaleparse PUBLIC kaleast)

add_library(kalehlir
  code/kalehlir/src/compile.cpp
  code/kalehlir/src/interpret.cpp
  code/kalehlir/src/symbol_table.cpp
  code/kalehlir/src/type.cpp
)
target_include_directories(kalehlir PUBLIC code/kalehlir/include)
target_link_libraries(kalehlir PUBLIC kaleast)

add_library(kalecli
  code/kale/run.cpp
)
target_include_directories(kalecli PUBLIC code/kale)
target_link_libraries(kalecli PUBLIC kalehlir kaleparse)

add_executable(kale
  code/kale/main.cpp
)
target_link_libraries(kale PRIVATE kalecli)

add_executable(kalelex-tests
  code/kalelex/test/lexeme_stream_test.cpp
  code/kalelex/test/ring_buffer_test.cpp
  code/kalelex/test/utf8_char_stream_test.cpp
)
target_link_libraries(kalelex-tests PRIVATE kalelex Catch2::Catch2WithMain)
target_compile_definitions(kalelex-tests PRIVATE EXAMPLES_PATH="${CMAKE_SOURCE_DIR}/examples")

add_executable(kaleparse-tests
  code/kaleparse/test/parser_test.cpp
)
target_link_libraries(kaleparse-tests PRIVATE kaleparse Catch2::Catch2WithMain)
target_compile_definitions(kaleparse-tests PRIVATE EXAMPLES_PATH="${CMAKE_SOURCE_DIR}/examples")

add_executable(kalehlir-tests
  code/kalehlir/test/symbol_table_test.cpp
  code/kalehlir/test/type_test.cpp
  code/kalehlir/test/compile_test.cpp
)
target_link_libraries(kalehlir-tests PRIVATE kalehlir kaleparse Catch2::Catch2WithMain)
target_compile_definitions(kalehlir-tests PRIVATE EXAMPLES_PATH="${CMAKE_SOURCE_DIR}/examples")

add_executable(kale-tests
  code/kale/test/run_test.cpp
)
target_link_libraries(kale-tests PRIVATE kalecli Catch2::Catch2WithMain)
