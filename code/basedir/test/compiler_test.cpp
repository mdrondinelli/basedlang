#include <fstream>
#include <sstream>
#include <string>

#include <catch2/catch_test_macros.hpp>

#include "basedir/compiler.h"
#include "basedlex/istream_binary_stream.h"
#include "basedlex/lexeme_stream.h"
#include "basedlex/lexeme_stream_reader.h"
#include "basedlex/utf8_char_stream.h"
#include "basedparse/parser.h"

static basedir::Program compile_source(std::string const &source)
{
  auto stream = std::istringstream{source};
  auto binary_stream = basedlex::Istream_binary_stream{&stream};
  auto char_stream = basedlex::Utf8_char_stream{&binary_stream};
  auto lexeme_stream = basedlex::Lexeme_stream{&char_stream};
  auto reader = basedlex::Lexeme_stream_reader{&lexeme_stream};
  auto parser = basedparse::Parser{&reader};
  auto const unit = parser.parse_translation_unit();
  auto compiler = basedir::Compiler{};
  return compiler.compile(*unit);
}

static basedir::Program compile_file(std::string const &filename)
{
  auto file = std::ifstream{std::string{EXAMPLES_PATH} + "/" + filename};
  auto source = std::string{
    std::istreambuf_iterator<char>{file},
    std::istreambuf_iterator<char>{}
  };
  return compile_source(source);
}

static basedir::Function_type const &
fn_type(basedir::Function const &fn)
{
  return std::get<basedir::Function_type>(fn.declaration.type->value);
}

TEST_CASE("Compiler - first.based produces one function")
{
  auto const program = compile_file("first.based");
  REQUIRE(program.functions.size() == 1);
  CHECK(program.functions[0].declaration.name == "main");
  CHECK(fn_type(program.functions[0]).parameter_types.empty());
  REQUIRE(program.functions[0].definition);
  CHECK(program.functions[0].definition->local_names.empty());
}

TEST_CASE("Compiler - parameters.based resolves parameters")
{
  auto const program = compile_file("parameters.based");
  REQUIRE(program.functions.size() == 3);
  CHECK(program.functions[0].declaration.name == "id");
  CHECK(fn_type(program.functions[0]).parameter_types.size() == 1);
  REQUIRE(program.functions[0].definition);
  CHECK(program.functions[0].definition->local_names.size() == 1);
  CHECK(program.functions[0].definition->local_names[0] == "x");
  CHECK(program.functions[1].declaration.name == "first");
  CHECK(fn_type(program.functions[1]).parameter_types.size() == 2);
  REQUIRE(program.functions[1].definition);
  CHECK(program.functions[1].definition->local_names.size() == 2);
}

TEST_CASE("Compiler - fibonacci.based compiles")
{
  auto const program = compile_file("fibonacci.based");
  REQUIRE(program.functions.size() == 2);
  CHECK(program.functions[0].declaration.name == "fib");
  CHECK(program.functions[1].declaration.name == "main");
}

TEST_CASE("Compiler - let creates local slots")
{
  auto const program = compile_source(R"(
    let f = fn() -> i32 {
      let x = 1;
      let y = 2;
      x + y
    };
  )");
  REQUIRE(program.functions.size() == 1);
  REQUIRE(program.functions[0].definition);
  CHECK(program.functions[0].definition->local_names.size() == 2);
  CHECK(program.functions[0].definition->local_names[0] == "x");
  CHECK(program.functions[0].definition->local_names[1] == "y");
}

TEST_CASE("Compiler - undefined symbol throws")
{
  CHECK_THROWS_AS(
    compile_source("let f = fn() -> i32 { x };"),
    basedir::Compiler::Compile_error
  );
}

TEST_CASE("Compiler - quicksort.based compiles")
{
  auto const program = compile_file("quicksort.based");
  REQUIRE(program.functions.size() == 3);
  CHECK(program.functions[0].declaration.name == "swap");
  CHECK(program.functions[1].declaration.name == "partition");
  CHECK(program.functions[2].declaration.name == "quicksort");
}
