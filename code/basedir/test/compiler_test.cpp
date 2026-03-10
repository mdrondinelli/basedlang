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

static basedir::Function_type const &fn_type(basedir::Function const &fn)
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
    let f = fn(): i32 -> {
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
    compile_source("let f = fn(): i32 -> { x };"),
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

// --- mutability ---

TEST_CASE("Compiler - assign to immutable let throws")
{
  CHECK_THROWS_AS(
    compile_source(R"(
      let f = fn(): i32 -> {
        let x = 1;
        x = 2;
        x
      };
    )"),
    basedir::Compiler::Compile_error
  );
}

TEST_CASE("Compiler - assign to mutable let compiles")
{
  CHECK_NOTHROW(compile_source(R"(
    let f = fn(): i32 -> {
      let mut x = 1;
      x = 2;
      x
    };
  )"));
}

TEST_CASE("Compiler - assign to immutable array element throws")
{
  CHECK_THROWS_AS(
    compile_source(R"(
      let f = fn(): i32 -> {
        let arr = new i32[3]{1, 2, 3};
        arr[0] = 99;
        arr[0]
      };
    )"),
    basedir::Compiler::Compile_error
  );
}

TEST_CASE("Compiler - assign to mutable array element compiles")
{
  CHECK_NOTHROW(compile_source(R"(
    let f = fn(): i32 -> {
      let mut arr = new i32[3]{1, 2, 3};
      arr[0] = 99;
      arr[0]
    };
  )"));
}

TEST_CASE("Compiler - assign to immutable parameter throws")
{
  CHECK_THROWS_AS(
    compile_source(R"(
      let f = fn(x: i32): i32 -> {
        x = 2;
        x
      };
    )"),
    basedir::Compiler::Compile_error
  );
}

TEST_CASE("Compiler - assign to mutable parameter compiles")
{
  CHECK_NOTHROW(compile_source(R"(
    let f = fn(mut x: i32): i32 -> {
      x = 2;
      x
    };
  )"));
}

TEST_CASE("Compiler - assign through mutable pointer compiles")
{
  CHECK_NOTHROW(compile_source(R"(
    let f = fn(p: i32 mut*): void -> {
      *p = 42;
    };
  )"));
}

TEST_CASE("Compiler - assign through pointer to immutable array throws")
{
  CHECK_THROWS_AS(
    compile_source(R"(
      let f = fn(): void -> {
        let arr = new i32[3]{1, 2, 3};
        let p = &arr;
        (*p)[0] = 99;
      };
    )"),
    basedir::Compiler::Compile_error
  );
}

TEST_CASE("Compiler - assign through pointer to mutable array compiles")
{
  CHECK_NOTHROW(compile_source(R"(
    let f = fn(): void -> {
      let mut arr = new i32[3]{1, 2, 3};
      let p = &arr;
      (*p)[0] = 99;
    };
  )"));
}

TEST_CASE("Compiler - nested mut ptr/array passes as all-immutable parameter")
{
  CHECK_NOTHROW(compile_source(R"(
    let read = fn(p: i32*[]*): i32 -> { *((*p)[0]) };
    let f = fn(): i32 -> {
      let mut x = 42;
      let mut arr = new i32 mut*[1]{&x};
      read(&arr)
    };
  )"));
}

TEST_CASE("Compiler - deep mut pointer chain passes as all-immutable")
{
  CHECK_NOTHROW(compile_source(R"(
    let read = fn(p: i32****): i32 -> { ****p };
    let f = fn(): i32 -> {
      let mut x = 42;
      let mut p1 = &x;
      let mut p2 = &p1;
      let mut p3 = &p2;
      let p4 = &p3;
      read(p4)
    };
  )"));
}

TEST_CASE("Compiler - immutable pointer cannot be passed as mut pointer")
{
  CHECK_THROWS_AS(
    compile_source(R"(
      let mutate = fn(p: i32[] mut*): void -> { (*p)[0] = 99; };
      let f = fn(): void -> {
        let arr = new i32[3]{1, 2, 3};
        mutate(&arr);
      };
    )"),
    basedir::Compiler::Compile_error
  );
}

TEST_CASE("Compiler - assign through immutable pointer throws")
{
  CHECK_THROWS_AS(
    compile_source(R"(
      let f = fn(p: i32*): void -> {
        *p = 42;
      };
    )"),
    basedir::Compiler::Compile_error
  );
}

// --- type checking: call arguments ---

TEST_CASE("Compiler - wrong argument count throws")
{
  CHECK_THROWS_AS(
    compile_source(R"(
      let f = fn(x: i32): i32 -> { x };
      let g = fn(): i32 -> { f(1, 2) };
    )"),
    basedir::Compiler::Compile_error
  );
}

TEST_CASE("Compiler - too few arguments throws")
{
  CHECK_THROWS_AS(
    compile_source(R"(
      let f = fn(x: i32, y: i32): i32 -> { x + y };
      let g = fn(): i32 -> { f(1) };
    )"),
    basedir::Compiler::Compile_error
  );
}

TEST_CASE("Compiler - wrong argument type throws")
{
  CHECK_THROWS_AS(
    compile_source(R"(
      let f = fn(x: i32): i32 -> { x };
      let g = fn(): i32 -> {
        let arr = new i32[1]{0};
        f(arr)
      };
    )"),
    basedir::Compiler::Compile_error
  );
}

TEST_CASE("Compiler - pointer where i32 expected throws")
{
  CHECK_THROWS_AS(
    compile_source(R"(
      let f = fn(x: i32): i32 -> { x };
      let g = fn(): i32 -> {
        let mut y = 0;
        f(&y)
      };
    )"),
    basedir::Compiler::Compile_error
  );
}

TEST_CASE("Compiler - i32 where pointer expected throws")
{
  CHECK_THROWS_AS(
    compile_source(R"(
      let f = fn(p: i32*): void -> { *p = 1; };
      let g = fn(): void -> { f(42) };
    )"),
    basedir::Compiler::Compile_error
  );
}

TEST_CASE("Compiler - sized array compatible with unsized parameter")
{
  CHECK_NOTHROW(compile_source(R"(
    let f = fn(arr: i32[]*): i32 -> { (*arr)[0] };
    let g = fn(): i32 -> {
      let mut arr = new i32[3]{10, 20, 30};
      f(&arr)
    };
  )"));
}

// --- type checking: assignment ---

TEST_CASE("Compiler - assign array to i32 throws")
{
  CHECK_THROWS_AS(
    compile_source(R"(
      let f = fn(): i32 -> {
        let mut x = 0;
        let arr = new i32[1]{1};
        x = arr;
        x
      };
    )"),
    basedir::Compiler::Compile_error
  );
}

TEST_CASE("Compiler - assign i32 to array element compiles")
{
  CHECK_NOTHROW(compile_source(R"(
    let f = fn(): i32 -> {
      let mut arr = new i32[3]{0, 0, 0};
      arr[0] = 42;
      arr[0]
    };
  )"));
}

// --- type checking: operators ---

TEST_CASE("Compiler - add array to array throws")
{
  CHECK_THROWS_AS(
    compile_source(R"(
      let f = fn(): i32 -> {
        let a = new i32[1]{1};
        let b = new i32[1]{2};
        a + b
      };
    )"),
    basedir::Compiler::Compile_error
  );
}

TEST_CASE("Compiler - compare pointer with i32 throws")
{
  CHECK_THROWS_AS(
    compile_source(R"(
      let f = fn(): i32 -> {
        let mut x = 0;
        &x < 1
      };
    )"),
    basedir::Compiler::Compile_error
  );
}

TEST_CASE("Compiler - negate array throws")
{
  CHECK_THROWS_AS(
    compile_source(R"(
      let f = fn(): i32 -> {
        let a = new i32[1]{1};
        -a
      };
    )"),
    basedir::Compiler::Compile_error
  );
}

TEST_CASE("Compiler - arithmetic on i32 compiles")
{
  CHECK_NOTHROW(compile_source(R"(
    let f = fn(): i32 -> { 1 + 2 * 3 - 4 / 2 % 3 };
  )"));
}

TEST_CASE("Compiler - comparison operators compile")
{
  CHECK_NOTHROW(compile_source(R"(
    let f = fn(a: i32, b: i32): i32 -> {
      if a < b { 0 - 1 }
      else if a > b { 1 }
      else if a <= b { 0 - 2 }
      else if a >= b { 2 }
      else if a == b { 0 }
      else { a != b }
    };
  )"));
}

// --- type checking: index ---

TEST_CASE("Compiler - index on i32 throws")
{
  CHECK_THROWS_AS(
    compile_source(R"(
      let f = fn(): i32 -> {
        let x = 42;
        x[0]
      };
    )"),
    basedir::Compiler::Compile_error
  );
}

TEST_CASE("Compiler - index with array as index throws")
{
  CHECK_THROWS_AS(
    compile_source(R"(
      let f = fn(): i32 -> {
        let arr = new i32[2]{1, 2};
        let idx = new i32[1]{0};
        arr[idx]
      };
    )"),
    basedir::Compiler::Compile_error
  );
}

// --- type checking: address-of and dereference ---

TEST_CASE("Compiler - address-of rvalue throws")
{
  CHECK_THROWS_AS(
    compile_source("let f = fn(): i32* -> { &42 };"),
    basedir::Compiler::Compile_error
  );
}

TEST_CASE("Compiler - address-of addition throws")
{
  CHECK_THROWS_AS(
    compile_source(R"(
      let f = fn(): i32* -> { &(1 + 2) };
    )"),
    basedir::Compiler::Compile_error
  );
}

TEST_CASE("Compiler - dereference i32 throws")
{
  CHECK_THROWS_AS(
    compile_source(R"(
      let f = fn(): i32 -> {
        let x = 42;
        *x
      };
    )"),
    basedir::Compiler::Compile_error
  );
}

TEST_CASE("Compiler - double pointer compiles")
{
  CHECK_NOTHROW(compile_source(R"(
    let f = fn(): i32 -> {
      let mut x = 42;
      let mut p = &x;
      let pp = &p;
      **pp
    };
  )"));
}

// --- type checking: constructor ---

TEST_CASE("Compiler - array constructor size mismatch throws")
{
  CHECK_THROWS_AS(
    compile_source(R"(
      let f = fn(): i32[2] -> { new i32[2]{1, 2, 3} };
    )"),
    basedir::Compiler::Compile_error
  );
}

TEST_CASE("Compiler - array constructor too few elements throws")
{
  CHECK_THROWS_AS(
    compile_source(R"(
      let f = fn(): i32[3] -> { new i32[3]{1} };
    )"),
    basedir::Compiler::Compile_error
  );
}

// --- scoping ---

TEST_CASE("Compiler - using a type name as value throws")
{
  CHECK_THROWS_AS(
    compile_source("let f = fn(): i32 -> { i32 };"),
    basedir::Compiler::Compile_error
  );
}

TEST_CASE("Compiler - unknown type throws")
{
  CHECK_THROWS_AS(
    compile_source("let f = fn(): i64 -> { 0 };"),
    basedir::Compiler::Compile_error
  );
}

TEST_CASE("Compiler - let shadowing compiles")
{
  auto const program = compile_source(R"(
    let f = fn(): i32 -> {
      let x = 1;
      let x = x + 1;
      x
    };
  )");
  REQUIRE(program.functions.size() == 1);
  REQUIRE(program.functions[0].definition);
  CHECK(program.functions[0].definition->local_names.size() == 2);
}

TEST_CASE("Compiler - block scoping hides inner bindings")
{
  CHECK_THROWS_AS(
    compile_source(R"(
      let f = fn(): i32 -> {
        if 1 { let y = 42; };
        y
      };
    )"),
    basedir::Compiler::Compile_error
  );
}

// --- function ---

TEST_CASE("Compiler - self-recursion compiles")
{
  CHECK_NOTHROW(compile_source(R"(
    let f = fn(n: i32): i32 -> {
      if n <= 0 { 0 }
      else { f(n - 1) }
    };
  )"));
}

TEST_CASE("Compiler - return i32 from void function throws")
{
  CHECK_THROWS_AS(
    compile_source("let f = fn(): void -> { 42 };"),
    basedir::Compiler::Compile_error
  );
}

TEST_CASE("Compiler - return array from i32 function throws")
{
  CHECK_THROWS_AS(
    compile_source(R"(
      let f = fn(): i32 -> { new i32[1]{1} };
    )"),
    basedir::Compiler::Compile_error
  );
}

TEST_CASE("Compiler - return statement type mismatch throws")
{
  CHECK_THROWS_AS(
    compile_source(R"(
      let f = fn(): i32 -> {
        let arr = new i32[1]{1};
        return arr;
      };
    )"),
    basedir::Compiler::Compile_error
  );
}

TEST_CASE("Compiler - correct return type compiles")
{
  CHECK_NOTHROW(compile_source(R"(
    let f = fn(): i32 -> { 42 };
  )"));
  CHECK_NOTHROW(compile_source(R"(
    let f = fn(): i32[2] -> { new i32[2]{1, 2} };
  )"));
  CHECK_NOTHROW(compile_source(R"(
    let f = fn(): i32 -> { return 42; };
  )"));
}

TEST_CASE("Compiler - void function with no tail compiles")
{
  CHECK_NOTHROW(compile_source(R"(
    let f = fn(): void -> {};
  )"));
  CHECK_NOTHROW(compile_source(R"(
    let f = fn(mut x: i32): void -> { x = 1; };
  )"));
}

TEST_CASE("Compiler - i32 function with only return statement compiles")
{
  CHECK_NOTHROW(compile_source(R"(
    let f = fn(x: i32): i32 -> {
      if x < 0 { return 0 - x; };
      return x;
    };
  )"));
}

TEST_CASE("Compiler - missing return type throws")
{
  CHECK_THROWS_AS(
    compile_source("let f = fn() -> 0;"),
    basedir::Compiler::Compile_error
  );
}

TEST_CASE("Compiler - calling non-function throws")
{
  CHECK_THROWS_AS(
    compile_source(R"(
      let f = fn(): i32 -> {
        let x = 42;
        x(1)
      };
    )"),
    basedir::Compiler::Compile_error
  );
}

TEST_CASE("Compiler - void function compiles")
{
  CHECK_NOTHROW(compile_source(R"(
    let f = fn(mut x: i32): void -> {
      x = x + 1;
    };
  )"));
}

// --- edge cases ---

TEST_CASE("Compiler - empty function body")
{
  CHECK_NOTHROW(compile_source(R"(
    let f = fn(): void -> {};
  )"));
}

TEST_CASE("Compiler - nested if-else compiles")
{
  CHECK_NOTHROW(compile_source(R"(
    let f = fn(x: i32): i32 -> {
      if x < 0 { 0 - 1 }
      else if x == 0 { 0 }
      else { 1 }
    };
  )"));
}

TEST_CASE("Compiler - while with complex body compiles")
{
  CHECK_NOTHROW(compile_source(R"(
    let f = fn(): i32 -> {
      let mut sum = 0;
      let mut i = 0;
      while i < 100 {
        if i % 2 == 0 {
          sum = sum + i;
        };
        i = i + 1;
      }
      sum
    };
  )"));
}

TEST_CASE("Compiler - return in middle of function compiles")
{
  CHECK_NOTHROW(compile_source(R"(
    let f = fn(x: i32): i32 -> {
      if x < 0 {
        return 0 - x;
      };
      x
    };
  )"));
}

TEST_CASE("Compiler - multiple functions can call each other forward")
{
  CHECK_THROWS_AS(
    compile_source(R"(
      let f = fn(): i32 -> { g() };
      let g = fn(): i32 -> { 42 };
    )"),
    basedir::Compiler::Compile_error
  );
}
