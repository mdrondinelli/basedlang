cmake_minimum_required(VERSION 3.22)
project(basedlang CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_compile_options(-Wall -Wextra -Wpedantic -Werror)

add_subdirectory(extern/Catch2-3.13.0)
add_subdirectory(extern/fmt)

add_library(basedlex
  code/basedlex/src/char_stream_reader.cpp
  code/basedlex/src/istream_binary_stream.cpp
  code/basedlex/src/lexeme_stream.cpp
  code/basedlex/src/lexeme_stream_reader.cpp
  code/basedlex/src/utf8_char_stream.cpp
)
target_include_directories(basedlex PUBLIC code/basedlex/include)

add_library(basedparse
  code/basedparse/src/ast.cpp
  code/basedparse/src/operator.cpp
  code/basedparse/src/parser.cpp
  code/basedparse/src/source_span.cpp
)
target_include_directories(basedparse PUBLIC code/basedparse/include)
target_link_libraries(basedparse PUBLIC basedlex)

add_library(basedhlir
  code/basedhlir/src/compile.cpp
  code/basedhlir/src/interpret.cpp
  code/basedhlir/src/symbol_table.cpp
  code/basedhlir/src/type.cpp
)
target_include_directories(basedhlir PUBLIC code/basedhlir/include)
target_link_libraries(basedhlir PUBLIC basedparse fmt)

add_library(basedcli
  code/based/run.cpp
)
target_include_directories(basedcli PUBLIC code/based)
target_link_libraries(basedcli PUBLIC basedhlir)

add_executable(based
  code/based/main.cpp
)
target_link_libraries(based PRIVATE basedcli)

add_executable(basedlex-tests
  code/basedlex/test/lexeme_stream_test.cpp
  code/basedlex/test/ring_buffer_test.cpp
  code/basedlex/test/utf8_char_stream_test.cpp
)
target_link_libraries(basedlex-tests PRIVATE basedlex Catch2::Catch2WithMain)
target_compile_definitions(basedlex-tests PRIVATE EXAMPLES_PATH="${CMAKE_SOURCE_DIR}/examples")

add_executable(basedparse-tests
  code/basedparse/test/parser_test.cpp
)
target_link_libraries(basedparse-tests PRIVATE basedparse Catch2::Catch2WithMain)
target_compile_definitions(basedparse-tests PRIVATE EXAMPLES_PATH="${CMAKE_SOURCE_DIR}/examples")

add_executable(basedhlir-tests
  code/basedhlir/test/symbol_table_test.cpp
  code/basedhlir/test/type_test.cpp
  code/basedhlir/test/compile_test.cpp
)
target_link_libraries(basedhlir-tests PRIVATE basedhlir Catch2::Catch2WithMain)
target_compile_definitions(basedhlir-tests PRIVATE EXAMPLES_PATH="${CMAKE_SOURCE_DIR}/examples")

add_executable(based-tests
  code/based/test/run_test.cpp
)
target_link_libraries(based-tests PRIVATE basedcli Catch2::Catch2WithMain)
