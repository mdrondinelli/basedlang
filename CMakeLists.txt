cmake_minimum_required(VERSION 3.22)
project(bensonlang CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_compile_options(-Wall -Wextra -Wpedantic -Werror)

add_subdirectory(extern/Catch2-3.13.0)

add_library(bensonlex
  code/bensonlex/src/char_stream_reader.cpp
  code/bensonlex/src/istream_binary_stream.cpp
  code/bensonlex/src/lexeme_stream.cpp
  code/bensonlex/src/lexeme_stream_reader.cpp
  code/bensonlex/src/utf8_char_stream.cpp
)
target_include_directories(bensonlex PUBLIC code/bensonlex/include)

add_library(bensonast
  code/bensonast/src/ast.cpp
  code/bensonast/src/operator.cpp
  code/bensonast/src/source_span.cpp
)
target_include_directories(bensonast PUBLIC code/bensonast/include)
target_link_libraries(bensonast PUBLIC bensonlex)

add_library(bensonparse
  code/bensonparse/src/parser.cpp
)
target_include_directories(bensonparse PUBLIC code/bensonparse/include)
target_link_libraries(bensonparse PUBLIC bensonast)

add_library(bensonhlir
  code/bensonhlir/src/compile.cpp
  code/bensonhlir/src/interpret.cpp
  code/bensonhlir/src/symbol_table.cpp
  code/bensonhlir/src/type.cpp
)
target_include_directories(bensonhlir PUBLIC code/bensonhlir/include)
target_link_libraries(bensonhlir PUBLIC bensonast)

add_library(bensoncli
  code/benson/run.cpp
)
target_include_directories(bensoncli PUBLIC code/benson)
target_link_libraries(bensoncli PUBLIC bensonhlir bensonparse)

add_executable(benson
  code/benson/main.cpp
)
target_link_libraries(benson PRIVATE bensoncli)

add_executable(bensonlex-tests
  code/bensonlex/test/lexeme_stream_test.cpp
  code/bensonlex/test/ring_buffer_test.cpp
  code/bensonlex/test/utf8_char_stream_test.cpp
)
target_link_libraries(bensonlex-tests PRIVATE bensonlex Catch2::Catch2WithMain)
target_compile_definitions(bensonlex-tests PRIVATE EXAMPLES_PATH="${CMAKE_SOURCE_DIR}/examples")

add_executable(bensonparse-tests
  code/bensonparse/test/parser_test.cpp
)
target_link_libraries(bensonparse-tests PRIVATE bensonparse Catch2::Catch2WithMain)
target_compile_definitions(bensonparse-tests PRIVATE EXAMPLES_PATH="${CMAKE_SOURCE_DIR}/examples")

add_executable(bensonhlir-tests
  code/bensonhlir/test/symbol_table_test.cpp
  code/bensonhlir/test/type_test.cpp
  code/bensonhlir/test/compile_test.cpp
)
target_link_libraries(bensonhlir-tests PRIVATE bensonhlir bensonparse Catch2::Catch2WithMain)
target_compile_definitions(bensonhlir-tests PRIVATE EXAMPLES_PATH="${CMAKE_SOURCE_DIR}/examples")

add_executable(benson-tests
  code/benson/test/run_test.cpp
)
target_link_libraries(benson-tests PRIVATE bensoncli Catch2::Catch2WithMain)
