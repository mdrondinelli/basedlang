cmake_minimum_required(VERSION 3.22)
project(bensonlang CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_compile_options(-Wall -Wextra -Wpedantic -Werror)

add_subdirectory(extern/Catch2-3.13.0)

add_library(text INTERFACE)
target_include_directories(text INTERFACE code/text/include)

add_library(lexing
  code/lexing/src/char_stream_reader.cpp
  code/lexing/src/istream_binary_stream.cpp
  code/lexing/src/lexeme_stream.cpp
  code/lexing/src/lexeme_stream_reader.cpp
  code/lexing/src/source_span.cpp
  code/lexing/src/utf8_char_stream.cpp
)
target_include_directories(lexing PUBLIC code/lexing/include)

add_library(ast
  code/ast/src/ast.cpp
  code/ast/src/operator.cpp
  code/ast/src/source_span.cpp
)
target_include_directories(ast PUBLIC code/ast/include)
target_link_libraries(ast PUBLIC lexing)

add_library(parsing
  code/parsing/src/parser.cpp
)
target_include_directories(parsing PUBLIC code/parsing/include)
target_link_libraries(parsing PUBLIC ast)

add_library(ir
  code/ir/src/compile.cpp
  code/ir/src/interpret.cpp
  code/ir/src/symbol_table.cpp
  code/ir/src/type.cpp
)
target_include_directories(ir PUBLIC code/ir/include)
target_link_libraries(ir PUBLIC ast)

add_library(bensoncli
  code/benson/run.cpp
)
target_include_directories(bensoncli PUBLIC code/benson)
target_link_libraries(bensoncli PUBLIC ir parsing)

add_executable(benson
  code/benson/main.cpp
)
target_link_libraries(benson PRIVATE bensoncli)

add_executable(lexing-tests
  code/lexing/test/lexeme_stream_test.cpp
  code/lexing/test/ring_buffer_test.cpp
  code/lexing/test/utf8_char_stream_test.cpp
)
target_link_libraries(lexing-tests PRIVATE lexing Catch2::Catch2WithMain)
target_compile_definitions(lexing-tests PRIVATE EXAMPLES_PATH="${CMAKE_SOURCE_DIR}/examples")

add_executable(text-tests
  code/text/test/string_table_test.cpp
)
target_link_libraries(text-tests PRIVATE text Catch2::Catch2WithMain)

add_executable(parsing-tests
  code/parsing/test/parser_test.cpp
)
target_link_libraries(parsing-tests PRIVATE parsing Catch2::Catch2WithMain)
target_compile_definitions(parsing-tests PRIVATE EXAMPLES_PATH="${CMAKE_SOURCE_DIR}/examples")

add_executable(ir-tests
  code/ir/test/symbol_table_test.cpp
  code/ir/test/type_test.cpp
  code/ir/test/compile_test.cpp
)
target_link_libraries(ir-tests PRIVATE ir parsing Catch2::Catch2WithMain)
target_compile_definitions(ir-tests PRIVATE EXAMPLES_PATH="${CMAKE_SOURCE_DIR}/examples")

add_executable(benson-tests
  code/benson/test/run_test.cpp
)
target_link_libraries(benson-tests PRIVATE bensoncli Catch2::Catch2WithMain)
