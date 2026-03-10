#include <cstdint>
#include <fstream>
#include <sstream>
#include <string>

#include <catch2/catch_test_macros.hpp>

#include "basedinterp/interpreter.h"
#include "basedir/compiler.h"
#include "basedlex/istream_binary_stream.h"
#include "basedlex/lexeme_stream.h"
#include "basedlex/lexeme_stream_reader.h"
#include "basedlex/utf8_char_stream.h"
#include "basedparse/parser.h"

struct Interp_fixture
{
  basedir::Program program;
  basedinterp::Interpreter interpreter;

  explicit Interp_fixture(basedir::Program prog)
      : program{std::move(prog)}, interpreter{&program}
  {
  }
};

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

static Interp_fixture from_source(std::string const &source)
{
  return Interp_fixture{compile_source(source)};
}

static Interp_fixture from_file(std::string const &filename)
{
  auto file = std::ifstream{std::string{EXAMPLES_PATH} + "/" + filename};
  auto source = std::string{
    std::istreambuf_iterator<char>{file},
    std::istreambuf_iterator<char>{}
  };
  return from_source(source);
}

TEST_CASE("Interpreter - first.based returns 0")
{
  auto fixture = from_file("first.based");
  auto const result = fixture.interpreter.call("main", {});
  auto const int_val = std::get_if<std::int32_t>(&result.data);
  REQUIRE(int_val);
  CHECK(*int_val == 0);
}

TEST_CASE("Interpreter - parameters.based returns 42")
{
  auto fixture = from_file("parameters.based");
  auto const result = fixture.interpreter.call("main", {});
  auto const int_val = std::get_if<std::int32_t>(&result.data);
  REQUIRE(int_val);
  CHECK(*int_val == 42);
}

TEST_CASE("Interpreter - fibonacci.based returns fib(10)")
{
  auto fixture = from_file("fibonacci.based");
  auto const result = fixture.interpreter.call("main", {});
  auto const int_val = std::get_if<std::int32_t>(&result.data);
  REQUIRE(int_val);
  CHECK(*int_val == 55);
}

TEST_CASE("Interpreter - call function with arguments from C++")
{
  auto fixture = from_source("let add = fn(a: i32, b: i32) -> i32 { a + b };");
  auto const result = fixture.interpreter.call(
    "add",
    {basedinterp::Value{std::int32_t{3}}, basedinterp::Value{std::int32_t{4}}}
  );
  auto const int_val = std::get_if<std::int32_t>(&result.data);
  REQUIRE(int_val);
  CHECK(*int_val == 7);
}

TEST_CASE("Interpreter - let shadowing")
{
  auto fixture = from_source(R"(
    let f = fn() -> i32 {
      let x = 1;
      let x = x + 1;
      x
    };
  )");
  auto const result = fixture.interpreter.call("f", {});
  auto const int_val = std::get_if<std::int32_t>(&result.data);
  REQUIRE(int_val);
  CHECK(*int_val == 2);
}

TEST_CASE("Interpreter - mutable variable assignment")
{
  auto fixture = from_source(R"(
    let f = fn() -> i32 {
      let mut x = 0;
      x = 10;
      x
    };
  )");
  auto const result = fixture.interpreter.call("f", {});
  auto const int_val = std::get_if<std::int32_t>(&result.data);
  REQUIRE(int_val);
  CHECK(*int_val == 10);
}

TEST_CASE("Interpreter - while loop")
{
  auto fixture = from_source(R"(
    let f = fn() -> i32 {
      let mut i = 0;
      let mut sum = 0;
      while i != 10 {
        i = i + 1;
        sum = sum + i;
      }
      sum
    };
  )");
  auto const result = fixture.interpreter.call("f", {});
  auto const int_val = std::get_if<std::int32_t>(&result.data);
  REQUIRE(int_val);
  CHECK(*int_val == 55);
}

TEST_CASE("Interpreter - if expression")
{
  auto fixture = from_source(R"(
    let abs = fn(x: i32) -> i32 {
      if x < 0 { 0 - x } else { x }
    };
  )");
  CHECK(
    std::get<std::int32_t>(fixture.interpreter
                             .call("abs", {basedinterp::Value{std::int32_t{5}}})
                             .data) == 5
  );
  CHECK(
    std::get<std::int32_t>(
      fixture.interpreter.call("abs", {basedinterp::Value{std::int32_t{-3}}})
        .data
    ) == 3
  );
  CHECK(
    std::get<std::int32_t>(fixture.interpreter
                             .call("abs", {basedinterp::Value{std::int32_t{0}}})
                             .data) == 0
  );
}

TEST_CASE("Interpreter - division by zero throws")
{
  auto fixture = from_source(R"(
    let f = fn() -> i32 { 1 / 0 };
  )");
  CHECK_THROWS_AS(
    fixture.interpreter.call("f", {}),
    basedinterp::Interpreter::Runtime_error
  );
}

TEST_CASE("Interpreter - array constructor and indexing")
{
  auto fixture = from_source(R"(
    let f = fn() -> i32 {
      let arr = new i32[3]{10, 20, 30};
      arr[0] + arr[1] + arr[2]
    };
  )");
  auto const result = fixture.interpreter.call("f", {});
  auto const int_val = std::get_if<std::int32_t>(&result.data);
  REQUIRE(int_val);
  CHECK(*int_val == 60);
}

TEST_CASE("Interpreter - array element assignment")
{
  auto fixture = from_source(R"(
    let f = fn() -> i32 {
      let mut arr = new i32[3]{1, 2, 3};
      arr[1] = 99;
      arr[1]
    };
  )");
  auto const result = fixture.interpreter.call("f", {});
  auto const int_val = std::get_if<std::int32_t>(&result.data);
  REQUIRE(int_val);
  CHECK(*int_val == 99);
}

TEST_CASE("Interpreter - array return value")
{
  auto fixture = from_source(R"(
    let f = fn() -> i32[3] {
      new i32[3]{10, 20, 30}
    };
  )");
  auto const result = fixture.interpreter.call("f", {});
  auto const arr = std::get_if<basedinterp::Array_value>(&result.data);
  REQUIRE(arr);
  REQUIRE(arr->elements.size() == 3);
  CHECK(std::get<std::int32_t>(arr->elements[0]->data) == 10);
  CHECK(std::get<std::int32_t>(arr->elements[1]->data) == 20);
  CHECK(std::get<std::int32_t>(arr->elements[2]->data) == 30);
}

TEST_CASE("Interpreter - array via pointer")
{
  auto fixture = from_source(R"(
    let set_first = fn(arr: i32[] mut*, val: i32) -> void {
      (*arr)[0] = val;
    };
    let f = fn() -> i32 {
      let mut arr = new i32[2]{0, 0};
      set_first(&arr, 42);
      arr[0]
    };
  )");
  auto const result = fixture.interpreter.call("f", {});
  auto const int_val = std::get_if<std::int32_t>(&result.data);
  REQUIRE(int_val);
  CHECK(*int_val == 42);
}

TEST_CASE("Interpreter - quicksort")
{
  auto fixture = from_source(R"(
    let swap = fn(arr: i32[] mut*, i: i32, j: i32) -> void {
      let tmp = (*arr)[i];
      (*arr)[i] = (*arr)[j];
      (*arr)[j] = tmp;
    };
    let partition = fn(arr: i32[] mut*, lo: i32, hi: i32) -> i32 {
      let pivot = (*arr)[hi];
      let mut i = lo - 1;
      let mut j = lo;
      while j < hi {
        if (*arr)[j] <= pivot {
          i = i + 1;
          swap(arr, i, j);
        };
        j = j + 1;
      }
      swap(arr, i + 1, hi);
      return i + 1;
    };
    let quicksort = fn(arr: i32[] mut*, lo: i32, hi: i32) -> void {
      if lo < hi {
        let p = partition(arr, lo, hi);
        quicksort(arr, lo, p - 1);
        quicksort(arr, p + 1, hi);
      };
    };
    let main = fn() -> i32[5] {
      let mut arr = new i32[5]{3, 1, 4, 1, 5};
      quicksort(&arr, 0, 4);
      arr
    };
  )");
  auto const result = fixture.interpreter.call("main", {});
  auto const arr = std::get_if<basedinterp::Array_value>(&result.data);
  REQUIRE(arr);
  REQUIRE(arr->elements.size() == 5);
  CHECK(std::get<std::int32_t>(arr->elements[0]->data) == 1);
  CHECK(std::get<std::int32_t>(arr->elements[1]->data) == 1);
  CHECK(std::get<std::int32_t>(arr->elements[2]->data) == 3);
  CHECK(std::get<std::int32_t>(arr->elements[3]->data) == 4);
  CHECK(std::get<std::int32_t>(arr->elements[4]->data) == 5);
}
