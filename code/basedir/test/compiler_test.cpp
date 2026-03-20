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

// --- new syntax: function body is any expression ---

TEST_CASE("Compiler - function body is a single literal")
{
  CHECK_NOTHROW(compile_source(R"(
    let f = fn(): i32 -> 42;
  )"));
}

TEST_CASE("Compiler - function body is an if expression")
{
  CHECK_NOTHROW(compile_source(R"(
    let abs = fn(x: i32): i32 -> if x < 0 { 0 - x } else { x };
  )"));
}

TEST_CASE("Compiler - function body is a parenthesized expression")
{
  CHECK_NOTHROW(compile_source(R"(
    let f = fn(x: i32): i32 -> (x + 1);
  )"));
}

TEST_CASE("Compiler - function body is a constructor")
{
  CHECK_NOTHROW(compile_source(R"(
    let f = fn(): i32[3] -> new i32[3]{1, 2, 3};
  )"));
}

TEST_CASE("Compiler - function body is a binary expression")
{
  CHECK_NOTHROW(compile_source(R"(
    let f = fn(a: i32, b: i32): i32 -> a + b;
  )"));
}

TEST_CASE("Compiler - function body is a function call")
{
  CHECK_NOTHROW(compile_source(R"(
    let id = fn(x: i32): i32 -> x;
    let f = fn(): i32 -> id(42);
  )"));
}

TEST_CASE("Compiler - non-block body with wrong return type throws")
{
  CHECK_THROWS_AS(
    compile_source(R"(
      let f = fn(): void -> 42;
    )"),
    basedir::Compiler::Compile_error
  );
}

// --- assignment edge cases ---

TEST_CASE("Compiler - assign to function call result throws")
{
  CHECK_THROWS_AS(
    compile_source(R"(
      let f = fn(): i32 -> { 42 };
      let g = fn(): i32 -> {
        f() = 1;
        0
      };
    )"),
    basedir::Compiler::Compile_error
  );
}

TEST_CASE("Compiler - assign to arithmetic result throws")
{
  CHECK_THROWS_AS(
    compile_source(R"(
      let f = fn(): i32 -> {
        let mut x = 1;
        let mut y = 2;
        (x + y) = 3;
        0
      };
    )"),
    basedir::Compiler::Compile_error
  );
}

TEST_CASE("Compiler - assign to literal throws")
{
  CHECK_THROWS_AS(
    compile_source(R"(
      let f = fn(): i32 -> {
        42 = 1;
        0
      };
    )"),
    basedir::Compiler::Compile_error
  );
}

TEST_CASE("Compiler - assign to if expression lvalue compiles")
{
  // if both branches yield a mutable reference, the if expression is an lvalue
  CHECK_NOTHROW(compile_source(R"(
    let f = fn(): i32 -> {
      let mut x = 1;
      let mut y = 2;
      (if 1 { x } else { y }) = 42;
      x
    };
  )"));
}

TEST_CASE("Compiler - assign to if expression with immutable branch throws")
{
  CHECK_THROWS_AS(
    compile_source(R"(
      let f = fn(): i32 -> {
        let x = 1;
        let mut y = 2;
        (if 1 { x } else { y }) = 42;
        0
      };
    )"),
    basedir::Compiler::Compile_error
  );
}

TEST_CASE("Compiler - chained assignment compiles")
{
  CHECK_NOTHROW(compile_source(R"(
    let f = fn(): i32 -> {
      let mut x = 0;
      let mut y = 0;
      x = y = 42;
      x + y
    };
  )"));
}

TEST_CASE("Compiler - assign pointer to i32 variable throws")
{
  CHECK_THROWS_AS(
    compile_source(R"(
      let f = fn(): i32 -> {
        let mut x = 0;
        let mut y = 1;
        x = &y;
        x
      };
    )"),
    basedir::Compiler::Compile_error
  );
}

TEST_CASE("Compiler - assign i32 to pointer variable throws")
{
  CHECK_THROWS_AS(
    compile_source(R"(
      let f = fn(): void -> {
        let mut x = 0;
        let mut p = &x;
        p = 42;
      };
    )"),
    basedir::Compiler::Compile_error
  );
}

// --- type checking: call result usage ---

TEST_CASE("Compiler - using void function result as i32 throws")
{
  CHECK_THROWS_AS(
    compile_source(R"(
      let noop = fn(): void -> {};
      let f = fn(): i32 -> { noop() + 1 };
    )"),
    basedir::Compiler::Compile_error
  );
}

TEST_CASE("Compiler - calling with zero args when function expects args throws")
{
  CHECK_THROWS_AS(
    compile_source(R"(
      let f = fn(x: i32): i32 -> { x };
      let g = fn(): i32 -> { f() };
    )"),
    basedir::Compiler::Compile_error
  );
}

TEST_CASE("Compiler - passing array where pointer expected throws")
{
  CHECK_THROWS_AS(
    compile_source(R"(
      let f = fn(p: i32*): i32 -> { *p };
      let g = fn(): i32 -> {
        let arr = new i32[1]{42};
        f(arr)
      };
    )"),
    basedir::Compiler::Compile_error
  );
}

TEST_CASE("Compiler - passing pointer where array expected throws")
{
  CHECK_THROWS_AS(
    compile_source(R"(
      let f = fn(arr: i32[]*): i32 -> { (*arr)[0] };
      let g = fn(): i32 -> {
        let mut x = 42;
        f(&x)
      };
    )"),
    basedir::Compiler::Compile_error
  );
}

// --- type checking: operators with more types ---

TEST_CASE("Compiler - add pointer + pointer throws")
{
  CHECK_THROWS_AS(
    compile_source(R"(
      let f = fn(): i32 -> {
        let mut x = 1;
        let mut y = 2;
        &x + &y
      };
    )"),
    basedir::Compiler::Compile_error
  );
}

TEST_CASE("Compiler - multiply pointer by i32 throws")
{
  CHECK_THROWS_AS(
    compile_source(R"(
      let f = fn(): i32 -> {
        let mut x = 1;
        &x * 2
      };
    )"),
    basedir::Compiler::Compile_error
  );
}

TEST_CASE("Compiler - compare array == array throws")
{
  CHECK_THROWS_AS(
    compile_source(R"(
      let f = fn(): i32 -> {
        let a = new i32[1]{1};
        let b = new i32[1]{1};
        a == b
      };
    )"),
    basedir::Compiler::Compile_error
  );
}

TEST_CASE("Compiler - modulo with void operand throws")
{
  CHECK_THROWS_AS(
    compile_source(R"(
      let noop = fn(): void -> {};
      let f = fn(): i32 -> { noop() % 2 };
    )"),
    basedir::Compiler::Compile_error
  );
}

TEST_CASE("Compiler - negate pointer throws")
{
  CHECK_THROWS_AS(
    compile_source(R"(
      let f = fn(): i32 -> {
        let mut x = 1;
        -&x
      };
    )"),
    basedir::Compiler::Compile_error
  );
}

TEST_CASE("Compiler - unary plus on array throws")
{
  CHECK_THROWS_AS(
    compile_source(R"(
      let f = fn(): i32 -> {
        let a = new i32[1]{1};
        +a
      };
    )"),
    basedir::Compiler::Compile_error
  );
}

// --- type checking: index edge cases ---

TEST_CASE("Compiler - index on pointer throws")
{
  CHECK_THROWS_AS(
    compile_source(R"(
      let f = fn(): i32 -> {
        let mut x = 42;
        let p = &x;
        p[0]
      };
    )"),
    basedir::Compiler::Compile_error
  );
}

TEST_CASE("Compiler - index on void throws")
{
  CHECK_THROWS_AS(
    compile_source(R"(
      let noop = fn(): void -> {};
      let f = fn(): i32 -> { noop()[0] };
    )"),
    basedir::Compiler::Compile_error
  );
}

TEST_CASE("Compiler - index with pointer as index throws")
{
  CHECK_THROWS_AS(
    compile_source(R"(
      let f = fn(): i32 -> {
        let arr = new i32[2]{1, 2};
        let mut idx = 0;
        arr[&idx]
      };
    )"),
    basedir::Compiler::Compile_error
  );
}

// --- type checking: address-of and dereference edge cases ---

TEST_CASE("Compiler - address-of array element compiles")
{
  CHECK_NOTHROW(compile_source(R"(
    let f = fn(): i32 -> {
      let mut arr = new i32[3]{10, 20, 30};
      let p = &arr[1];
      *p
    };
  )"));
}

TEST_CASE("Compiler - address-of dereference roundtrips")
{
  CHECK_NOTHROW(compile_source(R"(
    let f = fn(): i32 -> {
      let mut x = 42;
      let p = &x;
      let q = &*p;
      *q
    };
  )"));
}

TEST_CASE("Compiler - dereference array throws")
{
  CHECK_THROWS_AS(
    compile_source(R"(
      let f = fn(): i32 -> {
        let arr = new i32[2]{1, 2};
        *arr
      };
    )"),
    basedir::Compiler::Compile_error
  );
}

TEST_CASE("Compiler - address-of function throws")
{
  CHECK_THROWS_AS(
    compile_source(R"(
      let g = fn(): i32 -> { 42 };
      let f = fn(): i32 -> { &g; 0 };
    )"),
    basedir::Compiler::Compile_error
  );
}

TEST_CASE("Compiler - address-of constructor result throws")
{
  CHECK_THROWS_AS(
    compile_source(R"(
      let f = fn(): i32 -> { &new i32[1]{1}; 0 };
    )"),
    basedir::Compiler::Compile_error
  );
}

// --- constructor edge cases ---

// TODO: Detemine whether this should actually be okay.
TEST_CASE("Compiler - constructor for non-array type throws")
{
  CHECK_THROWS_AS(
    compile_source(R"(
      let f = fn(): i32 -> { new i32{1} };
    )"),
    basedir::Compiler::Compile_error
  );
}

// TODO: Determine whether this should actually be okay.
TEST_CASE("Compiler - zero-length array constructor compiles")
{
  CHECK_NOTHROW(compile_source(R"(
    let f = fn(): i32[0] -> { new i32[0]{} };
  )"));
}

TEST_CASE("Compiler - nested array constructor compiles")
{
  CHECK_NOTHROW(compile_source(R"(
    let f = fn(): i32[2] -> {
      let inner = new i32[2]{10, 20};
      inner
    };
  )"));
}

TEST_CASE("Compiler - constructor expression with array of dynamic size throws")
{
  CHECK_THROWS_AS(compile_source(R"(
    let f = fn(): i32[2] -> {
      let inner = new i32[]{};
      inner
    };
  )"),
  basedir::Compiler::Compile_error);
}

// --- scoping edge cases ---

TEST_CASE("Compiler - while body scoping hides inner bindings")
{
  CHECK_THROWS_AS(
    compile_source(R"(
      let f = fn(): i32 -> {
        let mut i = 1;
        while i > 0 {
          let secret = 42;
          i = 0;
        }
        secret
      };
    )"),
    basedir::Compiler::Compile_error
  );
}

TEST_CASE("Compiler - nested blocks scope correctly")
{
  CHECK_NOTHROW(compile_source(R"(
    let f = fn(): i32 -> {
      let x = {
        let a = 1;
        let b = 2;
        a + b
      };
      x
    };
  )"));
}

TEST_CASE("Compiler - inner block binding does not leak to outer")
{
  CHECK_THROWS_AS(
    compile_source(R"(
      let f = fn(): i32 -> {
        { let inner = 42; };
        inner
      };
    )"),
    basedir::Compiler::Compile_error
  );
}

TEST_CASE("Compiler - shadowing function name with let compiles")
{
  CHECK_NOTHROW(compile_source(R"(
    let f = fn(): i32 -> { 42 };
    let g = fn(): i32 -> {
      let f = 10;
      f
    };
  )"));
}

TEST_CASE("Compiler - shadowing function name makes it uncallable")
{
  CHECK_THROWS_AS(
    compile_source(R"(
      let f = fn(): i32 -> { 42 };
      let g = fn(): i32 -> {
        let f = 10;
        f()
      };
    )"),
    basedir::Compiler::Compile_error
  );
}

// --- return type edge cases ---

TEST_CASE("Compiler - return pointer from i32 function throws")
{
  CHECK_THROWS_AS(
    compile_source(R"(
      let f = fn(): i32 -> {
        let mut x = 42;
        &x
      };
    )"),
    basedir::Compiler::Compile_error
  );
}

TEST_CASE("Compiler - return i32 from pointer function throws")
{
  CHECK_THROWS_AS(
    compile_source(R"(
      let f = fn(): i32* -> { 42 };
    )"),
    basedir::Compiler::Compile_error
  );
}

// NOTE: The compiler currently does not reject i32 functions with void body
// (empty block or if-without-else), because it has no control flow analysis
// to prove that the function always returns via a return statement. This is
// a known limitation.

// TODO: Make the compiler guarantee that non-void functions always return a value of the correct type.

TEST_CASE("Compiler - empty block body in i32 function compiles (no flow analysis)")
{
  CHECK_NOTHROW(compile_source(R"(
    let f = fn(): i32 -> {};
  )"));
}

TEST_CASE("Compiler - return statement with pointer from i32 function throws")
{
  CHECK_THROWS_AS(
    compile_source(R"(
      let f = fn(): i32 -> {
        let mut x = 0;
        return &x;
      };
    )"),
    basedir::Compiler::Compile_error
  );
}

TEST_CASE("Compiler - return correct pointer type compiles")
{
  CHECK_NOTHROW(compile_source(R"(
    let f = fn(): i32 -> {
      let mut x = 42;
      let p = &x;
      *p
    };
  )"));
}


// TODO: This shouldn't compile. You can't return an array of unknown size on the stack.
TEST_CASE("Compiler - return sized array from unsized return type compiles")
{
  CHECK_NOTHROW(compile_source(R"(
    let f = fn(): i32[] -> { new i32[3]{1, 2, 3} };
  )"));
}

// TODO: See above TODO
TEST_CASE("Compiler - return unsized array from sized return type throws")
{
  CHECK_THROWS_AS(
    compile_source(R"(
      let make = fn(): i32[] -> { new i32[3]{1, 2, 3} };
      let f = fn(): i32[3] -> { make() };
    )"),
    basedir::Compiler::Compile_error
  );
}

// --- deeply nested and complex expressions ---

TEST_CASE("Compiler - deeply nested blocks compile")
{
  CHECK_NOTHROW(compile_source(R"(
    let f = fn(): i32 -> {
      {
        {
          {
            42
          }
        }
      }
    };
  )"));
}

TEST_CASE("Compiler - deeply nested if-else chain compiles")
{
  CHECK_NOTHROW(compile_source(R"(
    let classify = fn(x: i32): i32 -> {
      if x < 0 - 100 { 0 - 3 }
      else if x < 0 - 10 { 0 - 2 }
      else if x < 0 { 0 - 1 }
      else if x == 0 { 0 }
      else if x < 10 { 1 }
      else if x < 100 { 2 }
      else { 3 }
    };
  )"));
}

TEST_CASE("Compiler - if-else with mismatched branch types throws")
{
  CHECK_THROWS_AS(compile_source(R"(
    let f = fn(x: i32): i32 -> {
      if x < 0 { 0 }
      else { new i32[1]{1} }
    };
  )"),
  basedir::Compiler::Compile_error);
}

TEST_CASE("Compiler - complex while with nested if and multiple locals")
{
  CHECK_NOTHROW(compile_source(R"(
    let f = fn(): i32 -> {
      let mut sum = 0;
      let mut i = 0;
      while i < 100 {
        let even = i % 2 == 0;
        if even {
          let doubled = i * 2;
          sum = sum + doubled;
        };
        let next = i + 1;
        i = next;
      }
      sum
    };
  )"));
}

TEST_CASE("Compiler - many local variables compile")
{
  auto const program = compile_source(R"(
    let f = fn(): i32 -> {
      let a = 1;
      let b = 2;
      let c = 3;
      let d = 4;
      let e = 5;
      let f = 6;
      let g = 7;
      let h = 8;
      a + b + c + d + e + f + g + h
    };
  )");
  REQUIRE(program.functions[0].definition);
  CHECK(program.functions[0].definition->local_names.size() == 8);
}

// --- mutability through pointer chains ---

TEST_CASE("Compiler - address-of immutable variable produces immutable pointer")
{
  CHECK_THROWS_AS(
    compile_source(R"(
      let write = fn(p: i32 mut*): void -> { *p = 1; };
      let f = fn(): void -> {
        let x = 42;
        write(&x);
      };
    )"),
    basedir::Compiler::Compile_error
  );
}

TEST_CASE("Compiler - address-of mutable variable produces mutable pointer")
{
  CHECK_NOTHROW(compile_source(R"(
    let write = fn(p: i32 mut*): void -> { *p = 1; };
    let f = fn(): void -> {
      let mut x = 42;
      write(&x);
    };
  )"));
}

TEST_CASE("Compiler - double deref assign through mut pointer chain compiles")
{
  CHECK_NOTHROW(compile_source(R"(
    let f = fn(): i32 -> {
      let mut x = 10;
      let mut p = &x;
      let pp = &p;
      **pp
    };
  )"));
}

TEST_CASE(
  "Compiler - assign through double deref of all-mut pointer chain compiles"
)
{
  CHECK_NOTHROW(compile_source(R"(
    let f = fn(): void -> {
      let mut x = 10;
      let mut p = &x;
      let mut pp = &p;
      **pp = 20;
    };
  )"));
}

TEST_CASE(
  "Compiler - assign through double deref of immutable binding to mut pointer"
)
{
  // pp is immutable (can't reassign pp itself), but *pp gives a mutable
  // reference because &p produced a mut pointer (p is let mut). So **pp = 20
  // is valid: the mutability flows through the pointer types, not the binding.
  CHECK_NOTHROW(compile_source(R"(
    let f = fn(): void -> {
      let mut x = 10;
      let mut p = &x;
      let pp = &p;
      **pp = 20;
    };
  )"));
}

TEST_CASE(
  "Compiler - assign through double deref where inner binding is immutable"
)
{
  // p is immutable, so &p produces an immutable pointer (i32 mut* *).
  // *pp gives an immutable reference to i32 mut*. But when we strip the
  // reference to deref again, we see i32 mut* (the value), which is mutable.
  // The inner pointer's mutability is a property of the pointer value, not the
  // binding. So **pp = 20 is valid — the inner pointer was created from &x
  // where x is mutable.
  CHECK_NOTHROW(compile_source(R"(
    let f = fn(): void -> {
      let mut x = 10;
      let p = &x;
      let mut pp = &p;
      **pp = 20;
    };
  )"));
}

TEST_CASE("Compiler - cannot reassign through immutable pointer deref")
{
  // p is immutable, so &p produces an immutable pointer ((i32 mut*) *).
  // *pp dereferences through the immutable pointer → immutable reference.
  // Assignment to *pp should fail.
  CHECK_THROWS_AS(
    compile_source(R"(
      let f = fn(): void -> {
        let mut x = 10;
        let mut y = 20;
        let p = &x;
        let mut pp = &p;
        *pp = &y;
      };
    )"),
    basedir::Compiler::Compile_error
  );
}

TEST_CASE("Compiler - reassign through mutable pointer deref compiles")
{
  // p is mutable, so &p produces a mut pointer ((i32 mut*) mut*).
  // *pp dereferences through mut pointer → mutable reference. Assignment ok.
  CHECK_NOTHROW(compile_source(R"(
    let f = fn(): void -> {
      let mut x = 10;
      let mut y = 20;
      let mut p = &x;
      let pp = &p;
      *pp = &y;
    };
  )"));
}

// --- empty program ---

TEST_CASE("Compiler - empty program compiles")
{
  auto const program = compile_source("");
  CHECK(program.functions.empty());
}

// --- misc edge cases ---

TEST_CASE("Compiler - unused variable compiles")
{
  CHECK_NOTHROW(compile_source(R"(
    let f = fn(): i32 -> {
      let unused = 999;
      42
    };
  )"));
}

TEST_CASE("Compiler - let mut with no assignment compiles")
{
  CHECK_NOTHROW(compile_source(R"(
    let f = fn(): i32 -> {
      let mut x = 0;
      x
    };
  )"));
}

TEST_CASE("Compiler - nested function expression throws")
{
  CHECK_THROWS_AS(
    compile_source(R"(
      let f = fn(): i32 -> {
        let g = fn(): i32 -> { 42 };
        0
      };
    )"),
    basedir::Compiler::Compile_error
  );
}

TEST_CASE("Compiler - expression statement discards value")
{
  CHECK_NOTHROW(compile_source(R"(
    let f = fn(): void -> {
      1 + 2;
    };
  )"));
}

TEST_CASE("Compiler - while loop with pointer mutation compiles")
{
  CHECK_NOTHROW(compile_source(R"(
    let fill = fn(arr: i32[] mut*, len: i32, val: i32): void -> {
      let mut i = 0;
      while i < len {
        (*arr)[i] = val;
        i = i + 1;
      }
    };
  )"));
}

TEST_CASE("Compiler - multiple return statements with consistent types compiles")
{
  CHECK_NOTHROW(compile_source(R"(
    let clamp = fn(x: i32, lo: i32, hi: i32): i32 -> {
      if x < lo { return lo; };
      if x > hi { return hi; };
      return x;
    };
  )"));
}

TEST_CASE(
  "Compiler - return statement type mismatch in branch throws"
)
{
  CHECK_THROWS_AS(
    compile_source(R"(
      let f = fn(): i32 -> {
        let arr = new i32[1]{1};
        if 1 { return arr; };
        0
      };
    )"),
    basedir::Compiler::Compile_error
  );
}

TEST_CASE("Compiler - let initializer type is inferred correctly")
{
  CHECK_NOTHROW(compile_source(R"(
    let f = fn(): i32 -> {
      let arr = new i32[3]{1, 2, 3};
      let mut p = &arr;
      (*p)[0]
    };
  )"));
}

TEST_CASE("Compiler - reassign mutable pointer to different target compiles")
{
  CHECK_NOTHROW(compile_source(R"(
    let f = fn(): i32 -> {
      let mut a = 1;
      let mut b = 2;
      let mut p = &a;
      p = &b;
      *p
    };
  )"));
}

TEST_CASE("Compiler - assign mutable array pointer to immutable pointer throws")
{
  CHECK_THROWS_AS(
    compile_source(R"(
      let mutate = fn(p: i32 mut*): void -> { *p = 0; };
      let f = fn(): void -> {
        let x = 42;
        let p = &x;
        mutate(p);
      };
    )"),
    basedir::Compiler::Compile_error
  );
}

TEST_CASE("Compiler - single function program with complex logic compiles")
{
  CHECK_NOTHROW(compile_source(R"(
    let gcd = fn(mut a: i32, mut b: i32): i32 -> {
      while b != 0 {
        let t = b;
        b = a % b;
        a = t;
      }
      a
    };
  )"));
}

TEST_CASE("Compiler - many functions with cross-calls compile")
{
  auto const program = compile_source(R"(
    let a = fn(): i32 -> { 1 };
    let b = fn(): i32 -> { a() + 1 };
    let c = fn(): i32 -> { b() + a() };
    let d = fn(): i32 -> { c() + b() + a() };
  )");
  CHECK(program.functions.size() == 4);
}

TEST_CASE("Compiler - block expression as let initializer compiles")
{
  CHECK_NOTHROW(compile_source(R"(
    let f = fn(): i32 -> {
      let x = {
        let tmp = 10;
        tmp * 2
      };
      x + 1
    };
  )"));
}

TEST_CASE("Compiler - if expression as let initializer compiles")
{
  CHECK_NOTHROW(compile_source(R"(
    let f = fn(x: i32): i32 -> {
      let sign = if x > 0 { 1 } else if x < 0 { 0 - 1 } else { 0 };
      sign
    };
  )"));
}

TEST_CASE("Compiler - if without else has void type")
{
  CHECK_NOTHROW(compile_source(R"(
    let f = fn(mut x: i32): void -> {
      if x < 0 { x = 0 - x; };
    };
  )"));
}

// TODO: Control flow analysis
// NOTE: Same limitation as empty block body — no flow analysis to reject this.
TEST_CASE("Compiler - if without else in i32 function compiles (no flow analysis)")
{
  CHECK_NOTHROW(compile_source(R"(
    let f = fn(x: i32): i32 -> {
      if x > 0 { 1 }
    };
  )"));
}
