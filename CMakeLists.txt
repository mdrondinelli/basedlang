cmake_minimum_required(VERSION 3.22)
project(bensonlang CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_compile_options(-Wall -Wextra -Wpedantic -Werror)

add_subdirectory(extern/Catch2-3.13.0)

add_library(spelling INTERFACE)
target_include_directories(spelling INTERFACE code/spelling/include)

add_library(streams INTERFACE)
target_include_directories(streams INTERFACE code/streams/include)

add_library(lexing
  code/lexing/src/lexeme_stream.cpp
  code/lexing/src/lexeme_stream_reader.cpp
  code/lexing/src/source_span.cpp
)
target_include_directories(lexing PUBLIC code/lexing/include)
target_link_libraries(lexing PUBLIC spelling streams)

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
  code/ir/src/interpret.cpp
  code/ir/src/type.cpp
)
target_include_directories(ir PUBLIC code/ir/include)
target_link_libraries(ir PUBLIC spelling)

add_library(frontend
  code/frontend/src/compile.cpp
  code/frontend/src/symbol_table.cpp
)
target_include_directories(frontend PUBLIC code/frontend/include)
target_link_libraries(frontend PUBLIC ir ast spelling)

add_library(bytecode
  code/bytecode/src/bytecode_writer.cpp
  code/bytecode/src/module_builder.cpp
)
target_include_directories(bytecode PUBLIC code/bytecode/include)
target_link_libraries(bytecode PUBLIC streams spelling)

add_library(vm
  code/vm/src/vm.cpp
)
target_include_directories(vm PUBLIC code/vm/include)
target_link_libraries(vm PUBLIC bytecode ir)

add_library(bensoncli
  code/benson/run.cpp
)
target_include_directories(bensoncli PUBLIC code/benson)
target_link_libraries(bensoncli PUBLIC frontend parsing)

add_executable(benson
  code/benson/main.cpp
)
target_link_libraries(benson PRIVATE bensoncli)

add_executable(lexing-tests
  code/lexing/test/lexeme_stream_test.cpp
  code/lexing/test/ring_buffer_test.cpp
)
target_link_libraries(lexing-tests PRIVATE lexing Catch2::Catch2WithMain)
target_compile_definitions(lexing-tests PRIVATE EXAMPLES_PATH="${CMAKE_SOURCE_DIR}/examples")

add_executable(streams-tests
  code/streams/test/buffered_input_stream_test.cpp
  code/streams/test/char_input_stream_peeker_test.cpp
  code/streams/test/istream_input_stream_test.cpp
  code/streams/test/posix_input_stream_test.cpp
  code/streams/test/utf8_char_input_stream_test.cpp
)
target_link_libraries(streams-tests PRIVATE streams Catch2::Catch2WithMain)

add_executable(spelling-tests
  code/spelling/test/spelling_test.cpp
)
target_link_libraries(spelling-tests PRIVATE spelling Catch2::Catch2WithMain)

add_executable(parsing-tests
  code/parsing/test/parser_test.cpp
)
target_link_libraries(parsing-tests PRIVATE parsing Catch2::Catch2WithMain)
target_compile_definitions(parsing-tests PRIVATE EXAMPLES_PATH="${CMAKE_SOURCE_DIR}/examples")

add_executable(ir-tests
  code/ir/test/type_test.cpp
)
target_link_libraries(ir-tests PRIVATE ir Catch2::Catch2WithMain)

add_executable(bytecode-tests
  code/bytecode/test/bytecode_writer_test.cpp
  code/bytecode/test/module_builder_test.cpp
)
target_link_libraries(bytecode-tests PRIVATE bytecode Catch2::Catch2WithMain)

add_executable(vm-tests
  code/vm/test/vm_test.cpp
)
target_link_libraries(vm-tests PRIVATE vm Catch2::Catch2WithMain)

add_executable(frontend-tests
  code/frontend/test/compile_test.cpp
  code/frontend/test/symbol_table_test.cpp
)
target_link_libraries(frontend-tests PRIVATE frontend parsing Catch2::Catch2WithMain)
target_compile_definitions(frontend-tests PRIVATE EXAMPLES_PATH="${CMAKE_SOURCE_DIR}/examples")

add_executable(benson-tests
  code/benson/test/run_test.cpp
)
target_link_libraries(benson-tests PRIVATE bensoncli Catch2::Catch2WithMain)
