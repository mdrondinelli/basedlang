#include <array>
#include <exception>
#include <limits>
#include <string_view>
#include <utility>

#include <catch2/catch_test_macros.hpp>

#include "basedlex/istream_binary_stream.h"
#include "basedlex/lexeme_stream.h"
#include "basedlex/lexeme_stream_reader.h"
#include "basedlex/utf8_char_stream.h"

#include "basedparse/parser.h"

#include "basedhlir/compile.h"
#include "basedhlir/interpret.h"
#include "basedhlir/type.h"

struct Parse_fixture
{
  std::istringstream stream;
  basedlex::Istream_binary_stream binary_stream;
  basedlex::Utf8_char_stream char_stream;
  basedlex::Lexeme_stream lexeme_stream;
  basedlex::Lexeme_stream_reader reader;
  basedparse::Parser parser;

  explicit Parse_fixture(std::string source)
      : stream{std::move(source)},
        binary_stream{&stream},
        char_stream{&binary_stream},
        lexeme_stream{&char_stream},
        reader{&lexeme_stream},
        parser{&reader}
  {
  }

  Parse_fixture(Parse_fixture const &other) = delete;

  Parse_fixture &operator=(Parse_fixture const &other) = delete;
};

struct Compile_fixture
{
  basedhlir::Type_pool types;
  basedhlir::Compilation_context compiler;

  explicit Compile_fixture()
      : compiler{&types}
  {
  }

  Compile_fixture(Compile_fixture const &other) = delete;

  Compile_fixture &operator=(Compile_fixture const &other) = delete;
};

basedhlir::Constant_value evaluate_constant(
  basedhlir::Compilation_context &compiler,
  std::string_view source
)
{
  auto parse_fixture = Parse_fixture{std::string{source}};
  auto const expr = parse_fixture.parser.parse_expression();
  return compiler.evaluate_constant_expression(*expr);
}

basedhlir::Type *
compile_type(basedhlir::Compilation_context &compiler, std::string_view source)
{
  auto parse_fixture = Parse_fixture{std::string{source}};
  auto const expr = parse_fixture.parser.parse_expression();
  return compiler.compile_type_expression(*expr);
}

template <typename T>
T evaluate_constant_as(
  basedhlir::Compilation_context &compiler,
  std::string_view source
)
{
  return std::get<T>(evaluate_constant(compiler, source));
}

#define CHECK_CONSTANT(compiler, source, expected) \
  CHECK(evaluate_constant_as<decltype(expected)>((compiler), (source)) == (expected))

TEST_CASE("validate_int_literal")
{
  auto const value = basedhlir::validate_int_literal(
    "42",
    std::numeric_limits<std::uint64_t>::max()
  );
  REQUIRE(value.has_value());
  CHECK(*value == 42);
}

TEST_CASE("validate_int_literal - uint64 overflow")
{
  CHECK_FALSE(
    basedhlir::validate_int_literal(
      "18446744073709551616",
      std::numeric_limits<std::uint64_t>::max()
    )
      .has_value()
  );
}

TEST_CASE("evaluate_constant_expression - integer literal")
{
  auto fixture = Compile_fixture{};
  CHECK_CONSTANT(fixture.compiler, "1", 1);
}

TEST_CASE("evaluate_constant_expression - integer literal above int32 max")
{
  auto fixture = Compile_fixture{};
  CHECK_CONSTANT(fixture.compiler, "2147483648", std::int64_t{2147483648});
}

TEST_CASE("evaluate_constant_expression - unary minus allows int32 minimum")
{
  auto fixture = Compile_fixture{};
  CHECK_CONSTANT(fixture.compiler, "-2147483648", std::numeric_limits<std::int32_t>::min());
}

TEST_CASE("compile_int_literal - i8 literal")
{
  auto fixture = Compile_fixture{};
  CHECK_CONSTANT(fixture.compiler, "42i8", std::int8_t{42});
}

TEST_CASE("compile_int_literal - i16 literal")
{
  auto fixture = Compile_fixture{};
  CHECK_CONSTANT(fixture.compiler, "1000i16", std::int16_t{1000});
}

TEST_CASE("compile_int_literal - i32 literal with explicit suffix")
{
  auto fixture = Compile_fixture{};
  CHECK_CONSTANT(fixture.compiler, "42i32", 42);
}

TEST_CASE("compile_int_literal - i64 literal")
{
  auto fixture = Compile_fixture{};
  CHECK_CONSTANT(fixture.compiler, "42i64", std::int64_t{42});
}

TEST_CASE("compile_int_literal - unsuffixed literal above int32 max is int64")
{
  auto fixture = Compile_fixture{};
  CHECK_CONSTANT(fixture.compiler, "2147483648", std::int64_t{2147483648});
}

TEST_CASE("compile_int_literal - i8 max")
{
  auto fixture = Compile_fixture{};
  CHECK_CONSTANT(fixture.compiler, "127i8", std::numeric_limits<std::int8_t>::max());
}

TEST_CASE("compile_int_literal - i8 above max throws")
{
  auto fixture = Compile_fixture{};
  CHECK_THROWS_AS(
    evaluate_constant(fixture.compiler, "128i8"),
    basedhlir::Compilation_error
  );
}

TEST_CASE("compile_int_literal - unary minus i8 minimum")
{
  auto fixture = Compile_fixture{};
  CHECK_CONSTANT(fixture.compiler, "-128i8", std::numeric_limits<std::int8_t>::min());
}

TEST_CASE("compile_int_literal - i16 max")
{
  auto fixture = Compile_fixture{};
  CHECK_CONSTANT(fixture.compiler, "32767i16", std::numeric_limits<std::int16_t>::max());
}

TEST_CASE("compile_int_literal - unary minus i16 minimum")
{
  auto fixture = Compile_fixture{};
  CHECK_CONSTANT(fixture.compiler, "-32768i16", std::numeric_limits<std::int16_t>::min());
}

TEST_CASE("compile_int_literal - i64 max")
{
  auto fixture = Compile_fixture{};
  CHECK_CONSTANT(
    fixture.compiler,
    "9223372036854775807i64",
    std::numeric_limits<std::int64_t>::max()
  );
}

TEST_CASE("compile_int_literal - unary minus i64 minimum")
{
  auto fixture = Compile_fixture{};
  CHECK_CONSTANT(
    fixture.compiler,
    "-9223372036854775808i64",
    std::numeric_limits<std::int64_t>::min()
  );
}

TEST_CASE("evaluate_constant_expression - integer arithmetic")
{
  auto fixture = Compile_fixture{};
  CHECK_CONSTANT(fixture.compiler, "3 % 2 / (3 - 4) * 5 + 6", 1);
}

TEST_CASE("evaluate_constant_expression - unary plus")
{
  auto fixture = Compile_fixture{};
  CHECK_CONSTANT(fixture.compiler, "+42", 42);
}

TEST_CASE("evaluate_constant_expression - unary minus")
{
  auto fixture = Compile_fixture{};
  CHECK_CONSTANT(fixture.compiler, "-42", -42);
}

TEST_CASE("evaluate_constant_expression - nested arithmetic")
{
  auto fixture = Compile_fixture{};
  CHECK_CONSTANT(fixture.compiler, "(1 + 2) * 3", 9);
}

TEST_CASE("evaluate_constant_expression - integer comparisons")
{
  auto fixture = Compile_fixture{};
  CHECK_CONSTANT(fixture.compiler, "123 < 456", true);
  CHECK_CONSTANT(fixture.compiler, "123 > 456", false);
  CHECK_CONSTANT(fixture.compiler, "123 <= 456", true);
  CHECK_CONSTANT(fixture.compiler, "123 >= 456", false);
  CHECK_CONSTANT(fixture.compiler, "123 == 456", false);
  CHECK_CONSTANT(fixture.compiler, "123 != 456", true);
}

TEST_CASE("evaluate_constant_expression - if expression")
{
  auto fixture = Compile_fixture{};
  CHECK_CONSTANT(fixture.compiler, "if 1 < 2 { 10 } else { 20 }", 10);
}

TEST_CASE("evaluate_constant_expression - if expression takes else branch")
{
  auto fixture = Compile_fixture{};
  CHECK_CONSTANT(fixture.compiler, "if 1 > 2 { 10 } else { 20 }", 20);
}

TEST_CASE("evaluate_constant_expression - if expression takes else-if branch")
{
  auto fixture = Compile_fixture{};
  CHECK_CONSTANT(
    fixture.compiler,
    "if 1 > 2 { 10 } else if 1 < 2 { 30 } else { 20 }",
    30
  );
}

TEST_CASE(
  "evaluate_constant_expression - if with constant true condition folds to "
  "then branch"
)
{
  auto fixture = Compile_fixture{};
  CHECK_CONSTANT(fixture.compiler, "if 1 < 2 { 10 }", 10);
}

TEST_CASE(
  "evaluate_constant_expression - if with constant true condition "
  "allows different branch types"
)
{
  auto fixture = Compile_fixture{};
  CHECK_CONSTANT(fixture.compiler, "if true { 10 } else { false }", 10);
}

TEST_CASE("compile_type_expression - sized array")
{
  auto fixture = Compile_fixture{};
  auto const type = compile_type(fixture.compiler, "[4]Int32");
  auto const sa = std::get_if<basedhlir::Sized_array_type>(&type->data);
  REQUIRE(sa != nullptr);
  CHECK(sa->element == fixture.types.int32_type());
  CHECK(sa->size == 4);
}

TEST_CASE("compile_type_expression - sized array with constant expression size")
{
  auto fixture = Compile_fixture{};
  auto const type = compile_type(fixture.compiler, "[2 + 2]Int32");
  auto const sa = std::get_if<basedhlir::Sized_array_type>(&type->data);
  REQUIRE(sa != nullptr);
  CHECK(sa->element == fixture.types.int32_type());
  CHECK(sa->size == 4);
}

TEST_CASE("compile_type_expression - sized array rejects zero size")
{
  auto fixture = Compile_fixture{};
  CHECK_THROWS_AS(
    compile_type(fixture.compiler, "[0]Int32"),
    basedhlir::Compilation_error
  );
}

TEST_CASE("compile_type_expression - sized array rejects negative size")
{
  auto fixture = Compile_fixture{};
  CHECK_THROWS_AS(
    compile_type(fixture.compiler, "[-1]Int32"),
    basedhlir::Compilation_error
  );
}

TEST_CASE("compile_type_expression - sized array rejects non-integer size")
{
  auto fixture = Compile_fixture{};
  CHECK_THROWS_AS(
    compile_type(fixture.compiler, "[1 < 2]Int32"),
    basedhlir::Compilation_error
  );
}

TEST_CASE("evaluate_constant_expression - bool equality")
{
  auto fixture = Compile_fixture{};
  CHECK_CONSTANT(fixture.compiler, "1 == 1 == (2 == 2)", true);
  CHECK_CONSTANT(fixture.compiler, "1 == 1 != (1 == 2)", true);
  CHECK_CONSTANT(fixture.compiler, "1 == 2 == (3 == 4)", true);
}

// --- Function compilation and interpretation tests ---

std::pair<basedhlir::Type_pool, basedhlir::Translation_unit>
compile_program(std::string source)
{
  auto parse_fixture = Parse_fixture{std::move(source)};
  auto const ast = parse_fixture.parser.parse_translation_unit();
  auto types = basedhlir::Type_pool{};
  auto tu = basedhlir::compile(ast, &types);
  return {std::move(types), std::move(tu)};
}

TEST_CASE("compile - function returning literal")
{
  auto const [types, tu] =
    compile_program("let f = fn(): Int32 => { return 0; };");
  REQUIRE(tu.functions.size() == 1);
  auto const &f = *tu.functions[0];
  REQUIRE(!f.blocks.empty());
  CHECK(f.blocks[0]->parameters.empty());
  auto const result = basedhlir::interpret(f, {});
  REQUIRE(std::holds_alternative<std::int32_t>(result));
  CHECK(std::get<std::int32_t>(result) == 0);
}

TEST_CASE("compile - function with parameter")
{
  auto const [types, tu] =
    compile_program("let id = fn(x: Int32): Int32 => { return x; };");
  REQUIRE(tu.functions.size() == 1);
  auto const &f = *tu.functions[0];
  REQUIRE(!f.blocks.empty());
  CHECK(f.blocks[0]->parameters.size() == 1);
  auto const args = std::array<basedhlir::Constant_value, 1>{std::int32_t{42}};
  auto const result = basedhlir::interpret(f, args);
  REQUIRE(std::holds_alternative<std::int32_t>(result));
  CHECK(std::get<std::int32_t>(result) == 42);
}

TEST_CASE("compile - function with arithmetic")
{
  auto const [types, tu] =
    compile_program("let add1 = fn(x: Int32): Int32 => { return x + 1; };");
  REQUIRE(tu.functions.size() == 1);
  auto const args = std::array<basedhlir::Constant_value, 1>{std::int32_t{41}};
  auto const result = basedhlir::interpret(*tu.functions[0], args);
  REQUIRE(std::holds_alternative<std::int32_t>(result));
  CHECK(std::get<std::int32_t>(result) == 42);
}

TEST_CASE("compile - function with multiple parameters")
{
  auto const [types, tu] = compile_program(
    "let first = fn(x: Int32, y: Int32): Int32 => { return x; };"
  );
  REQUIRE(tu.functions.size() == 1);
  auto const args =
    std::array<basedhlir::Constant_value, 2>{std::int32_t{1}, std::int32_t{2}};
  auto const result = basedhlir::interpret(*tu.functions[0], args);
  REQUIRE(std::holds_alternative<std::int32_t>(result));
  CHECK(std::get<std::int32_t>(result) == 1);
}

TEST_CASE("compile - function with if expression")
{
  auto const [types, tu] = compile_program(
    "let abs = fn(x: Int32): Int32 => "
    "{ return if x < 0 { 0 - x } else { x }; };"
  );
  REQUIRE(tu.functions.size() == 1);
  auto const &f = *tu.functions[0];
  auto const args1 = std::array<basedhlir::Constant_value, 1>{std::int32_t{-5}};
  auto const r1 = basedhlir::interpret(f, args1);
  CHECK(std::get<std::int32_t>(r1) == 5);
  auto const args2 = std::array<basedhlir::Constant_value, 1>{std::int32_t{3}};
  auto const r2 = basedhlir::interpret(f, args2);
  CHECK(std::get<std::int32_t>(r2) == 3);
}

TEST_CASE("compile - function with block and let bindings")
{
  auto const [types, tu] = compile_program(
    "let f = fn(): Int32 => { let x = 5; let y = 10; return x + y; };"
  );
  REQUIRE(tu.functions.size() == 1);
  auto const result = basedhlir::interpret(*tu.functions[0], {});
  REQUIRE(std::holds_alternative<std::int32_t>(result));
  CHECK(std::get<std::int32_t>(result) == 15);
}

TEST_CASE("compile - function with tail expression")
{
  auto const [types, tu] = compile_program("let f = fn(): Int32 => { 42 };");
  REQUIRE(tu.functions.size() == 1);
  auto const result = basedhlir::interpret(*tu.functions[0], {});
  REQUIRE(std::holds_alternative<std::int32_t>(result));
  CHECK(std::get<std::int32_t>(result) == 42);
}

TEST_CASE("compile - function calling another function")
{
  auto const [types, tu] = compile_program(
    "let id = fn(x: Int32): Int32 => { return x; };"
    "let main = fn(): Int32 => { return id(42); };"
  );
  REQUIRE(tu.functions.size() == 2);
  auto const result = basedhlir::interpret(*tu.functions[1], {});
  REQUIRE(std::holds_alternative<std::int32_t>(result));
  CHECK(std::get<std::int32_t>(result) == 42);
}

TEST_CASE("compile - nested function calls")
{
  auto const [types, tu] = compile_program(
    "let id = fn(x: Int32): Int32 => { return x; };"
    "let first = fn(x: Int32, y: Int32): Int32 => { return x; };"
    "let main = fn(): Int32 => { return first(id(42), 0); };"
  );
  REQUIRE(tu.functions.size() == 3);
  auto const result = basedhlir::interpret(*tu.functions[2], {});
  REQUIRE(std::holds_alternative<std::int32_t>(result));
  CHECK(std::get<std::int32_t>(result) == 42);
}

TEST_CASE("compile - recursive function")
{
  auto const [types, tu] = compile_program(
    "let fib = fn(n: Int32): Int32 => "
    "{ return if n == 0 { 0 } else if n == 1 { 1 } "
    "else { recurse(n - 1) + recurse(n - 2) }; };"
  );
  REQUIRE(tu.functions.size() == 1);
  auto const args = std::array<basedhlir::Constant_value, 1>{std::int32_t{10}};
  auto const result = basedhlir::interpret(*tu.functions[0], args);
  REQUIRE(std::holds_alternative<std::int32_t>(result));
  CHECK(std::get<std::int32_t>(result) == 55);
}

TEST_CASE("compile - recurse keyword")
{
  auto const [types, tu] = compile_program(
    "let factorial = fn(n: Int32): Int32 => "
    "{ return if n == 0 { 1 } else { n * recurse(n - 1) }; };"
  );
  REQUIRE(tu.functions.size() == 1);
  auto const args = std::array<basedhlir::Constant_value, 1>{std::int32_t{5}};
  auto const result = basedhlir::interpret(*tu.functions[0], args);
  REQUIRE(std::holds_alternative<std::int32_t>(result));
  CHECK(std::get<std::int32_t>(result) == 120);
}

TEST_CASE("compile - compile-time function call evaluation")
{
  auto const [types, tu] = compile_program(
    "let add = fn(x: Int32, y: Int32): Int32 => { return x + y; };"
    "let result = add(3, 4);"
  );
  // add is compiled as a function, result is evaluated at compile time
  CHECK(tu.functions.size() == 1);
}

TEST_CASE("compile - fuel exhaustion")
{
  auto const [types, tu] = compile_program(
    "let loop = fn(n: Int32): Int32 => { return recurse(n); };"
  );
  REQUIRE(tu.functions.size() == 1);
  auto const args = std::array<basedhlir::Constant_value, 1>{std::int32_t{0}};
  auto fuel = std::int32_t{100};
  CHECK_THROWS_AS(
    basedhlir::interpret(*tu.functions[0], args, fuel),
    basedhlir::Fuel_exhausted_error
  );
}

TEST_CASE("Fuel_exhausted_error - std::exception contract")
{
  auto const error = basedhlir::Fuel_exhausted_error{};

  CHECK(
    std::string_view{error.what()} == "compile-time evaluation ran out of fuel"
  );

  try
  {
    throw error;
  }
  catch (std::exception const &caught)
  {
    CHECK(
      std::string_view{caught.what()} ==
      "compile-time evaluation ran out of fuel"
    );
    return;
  }

  FAIL("Fuel_exhausted_error was not catchable as std::exception");
}

TEST_CASE("compile - pure expression body")
{
  auto const [types, tu] = compile_program("let pi = fn(): Int32 => 3;");
  REQUIRE(tu.functions.size() == 1);
  auto const result = basedhlir::interpret(*tu.functions[0], {});
  REQUIRE(std::holds_alternative<std::int32_t>(result));
  CHECK(std::get<std::int32_t>(result) == 3);
}

TEST_CASE("compile - return type mismatch")
{
  CHECK_THROWS_AS(
    compile_program("let f = fn(): Int32 => { return true; };"),
    basedhlir::Compilation_failure
  );
}

// --- Compilation error tests ---

TEST_CASE("compile - branch type mismatch")
{
  CHECK_THROWS_AS(
    compile_program(
      "let f = fn(b: Bool): Int32 => { return if b { 1 } else { b }; };"
    ),
    basedhlir::Compilation_failure
  );
}

TEST_CASE("compile - else-if branch type mismatch")
{
  CHECK_THROWS_AS(
    compile_program(
      "let f = fn(b: Bool): Int32 => "
      "{ return if b { 1 } else if b { b } else { 2 }; };"
    ),
    basedhlir::Compilation_failure
  );
}

TEST_CASE("compile - wrong argument count")
{
  CHECK_THROWS_AS(
    compile_program(
      "let f = fn(x: Int32): Int32 => { return x; };"
      "let g = fn(): Int32 => { return f(1, 2); };"
    ),
    basedhlir::Compilation_failure
  );
}

TEST_CASE("compile - non-bool if condition")
{
  CHECK_THROWS_AS(
    compile_program(
      "let f = fn(n: Int32): Int32 =>"
      "{ return if n { 1 } else { 0 }; };"
    ),
    basedhlir::Compilation_failure
  );
}

TEST_CASE("compile - wrong argument type")
{
  CHECK_THROWS_AS(
    compile_program(
      "let f = fn(x: Int32): Int32 => { return x; };"
      "let g = fn(): Int32 => { return f(true); };"
    ),
    basedhlir::Compilation_failure
  );
}

TEST_CASE("compile - calling non-callable")
{
  CHECK_THROWS_AS(
    compile_program("let f = fn(): Int32 => { return 3(); };"),
    basedhlir::Compilation_failure
  );
}

TEST_CASE("compile - undefined identifier")
{
  CHECK_THROWS_AS(
    compile_program("let f = fn(): Int32 => { return x; };"),
    basedhlir::Compilation_failure
  );
}

TEST_CASE("compile - top-level mutable binding")
{
  CHECK_THROWS_AS(
    compile_program("let mut x = 1;"),
    basedhlir::Compilation_failure
  );
}

TEST_CASE("compile - positive integer literal above int32 max")
{
  CHECK_THROWS_AS(
    compile_program("let f = fn(): Int32 => { return 2147483648; };"),
    basedhlir::Compilation_failure
  );
}

TEST_CASE("compile - unary minus integer literal below int32 min")
{
  CHECK_THROWS_AS(
    compile_program("let f = fn(): Int32 => { return -2147483649; };"),
    basedhlir::Compilation_failure
  );
}

TEST_CASE("compile - top-level fuel exhaustion")
{
  CHECK_THROWS_AS(
    compile_program(
      "let f = fn(x: Int32): Int32 =>"
      "{ return if x < 1 { 0 } else { recurse(x - 1) + recurse(x - 1) }; };"
      "let y = f(30);"
    ),
    basedhlir::Compilation_failure
  );
}

// --- CFG-specific behavior tests ---

TEST_CASE("compile - void if expression")
{
  auto const [types, tu] =
    compile_program("let f = fn(x: Int32): Void => { if x < 0 { 0 - x; }; };");
  REQUIRE(tu.functions.size() == 1);
  auto const args = std::array<basedhlir::Constant_value, 1>{std::int32_t{-5}};
  auto const result = basedhlir::interpret(*tu.functions[0], args);
  CHECK(std::holds_alternative<basedhlir::Void_value>(result));
}

TEST_CASE("compile - early return")
{
  auto const [types, tu] =
    compile_program("let f = fn(x: Int32): Int32 => { return 1; return 2; };");
  REQUIRE(tu.functions.size() == 1);
  auto const args = std::array<basedhlir::Constant_value, 1>{std::int32_t{0}};
  auto const result = basedhlir::interpret(*tu.functions[0], args);
  REQUIRE(std::holds_alternative<std::int32_t>(result));
  CHECK(std::get<std::int32_t>(result) == 1);
}

TEST_CASE("compile - if expression in tail position")
{
  auto const [types, tu] = compile_program(
    "let f = fn(x: Int32): Int32 => { if x < 0 { 0 } else { x } };"
  );
  REQUIRE(tu.functions.size() == 1);
  auto const args1 = std::array<basedhlir::Constant_value, 1>{std::int32_t{-3}};
  auto const r1 = basedhlir::interpret(*tu.functions[0], args1);
  REQUIRE(std::holds_alternative<std::int32_t>(r1));
  CHECK(std::get<std::int32_t>(r1) == 0);
  auto const args2 = std::array<basedhlir::Constant_value, 1>{std::int32_t{7}};
  auto const r2 = basedhlir::interpret(*tu.functions[0], args2);
  REQUIRE(std::holds_alternative<std::int32_t>(r2));
  CHECK(std::get<std::int32_t>(r2) == 7);
}

// --- Deeper feature coverage ---

TEST_CASE("compile - nested if expressions")
{
  auto const [types, tu] = compile_program(
    "let f = fn(a: Int32, b: Int32): Int32 => "
    "{ return if a < 0 { if b < 0 { 1 } else { 2 } } else { 3 }; };"
  );
  REQUIRE(tu.functions.size() == 1);
  auto const &f = *tu.functions[0];
  auto const args1 = std::array<basedhlir::Constant_value, 2>{
    std::int32_t{-1},
    std::int32_t{-1}
  };
  CHECK(std::get<std::int32_t>(basedhlir::interpret(f, args1)) == 1);
  auto const args2 =
    std::array<basedhlir::Constant_value, 2>{std::int32_t{-1}, std::int32_t{1}};
  CHECK(std::get<std::int32_t>(basedhlir::interpret(f, args2)) == 2);
  auto const args3 =
    std::array<basedhlir::Constant_value, 2>{std::int32_t{1}, std::int32_t{0}};
  CHECK(std::get<std::int32_t>(basedhlir::interpret(f, args3)) == 3);
}

TEST_CASE("compile - multiple else-if branches")
{
  auto const [types, tu] = compile_program(
    "let f = fn(x: Int32): Int32 => "
    "{ return if x == 1 { 10 } else if x == 2 { 20 } "
    "else if x == 3 { 30 } else { 0 }; };"
  );
  REQUIRE(tu.functions.size() == 1);
  auto const &f = *tu.functions[0];
  auto const a1 = std::array<basedhlir::Constant_value, 1>{std::int32_t{1}};
  CHECK(std::get<std::int32_t>(basedhlir::interpret(f, a1)) == 10);
  auto const a2 = std::array<basedhlir::Constant_value, 1>{std::int32_t{2}};
  CHECK(std::get<std::int32_t>(basedhlir::interpret(f, a2)) == 20);
  auto const a3 = std::array<basedhlir::Constant_value, 1>{std::int32_t{3}};
  CHECK(std::get<std::int32_t>(basedhlir::interpret(f, a3)) == 30);
  auto const a4 = std::array<basedhlir::Constant_value, 1>{std::int32_t{99}};
  CHECK(std::get<std::int32_t>(basedhlir::interpret(f, a4)) == 0);
}

TEST_CASE("compile - block scoping")
{
  auto const [types, tu] = compile_program(
    "let f = fn(): Int32 => { let x = 1; return { let x = 2; x } + x; };"
  );
  REQUIRE(tu.functions.size() == 1);
  auto const result = basedhlir::interpret(*tu.functions[0], {});
  REQUIRE(std::holds_alternative<std::int32_t>(result));
  CHECK(std::get<std::int32_t>(result) == 3);
}

TEST_CASE("compile - expression statement")
{
  auto const [types, tu] =
    compile_program("let f = fn(x: Int32): Int32 => { 0 - x; return x; };");
  REQUIRE(tu.functions.size() == 1);
  auto const args = std::array<basedhlir::Constant_value, 1>{std::int32_t{42}};
  auto const result = basedhlir::interpret(*tu.functions[0], args);
  REQUIRE(std::holds_alternative<std::int32_t>(result));
  CHECK(std::get<std::int32_t>(result) == 42);
}

// --- Float literal tests ---

TEST_CASE("compile_float_literal - unsuffixed defaults to Float64")
{
  auto fixture = Compile_fixture{};
  CHECK_CONSTANT(fixture.compiler, "3.14", 3.14);
}

TEST_CASE("compile_float_literal - f suffix gives Float32")
{
  auto fixture = Compile_fixture{};
  CHECK_CONSTANT(fixture.compiler, "1.5f", 1.5f);
}

TEST_CASE("compile_float_literal - d suffix gives Float64")
{
  auto fixture = Compile_fixture{};
  CHECK_CONSTANT(fixture.compiler, "1.5d", 1.5);
}

TEST_CASE("compile_float_literal - unary minus")
{
  auto fixture = Compile_fixture{};
  CHECK_CONSTANT(fixture.compiler, "-2.5", -2.5);
}

TEST_CASE("compile_float_literal - unary plus")
{
  auto fixture = Compile_fixture{};
  CHECK_CONSTANT(fixture.compiler, "+2.5", 2.5);
}

TEST_CASE("evaluate_constant_expression - float arithmetic")
{
  auto fixture = Compile_fixture{};
  CHECK_CONSTANT(fixture.compiler, "1.0 + 2.0", 3.0);
  CHECK_CONSTANT(fixture.compiler, "5.0 - 1.5", 3.5);
  CHECK_CONSTANT(fixture.compiler, "2.0 * 3.0", 6.0);
  CHECK_CONSTANT(fixture.compiler, "10.0 / 4.0", 2.5);
}

TEST_CASE("evaluate_constant_expression - float comparisons")
{
  auto fixture = Compile_fixture{};
  CHECK_CONSTANT(fixture.compiler, "1.0 < 2.0", true);
  CHECK_CONSTANT(fixture.compiler, "1.0 > 2.0", false);
  CHECK_CONSTANT(fixture.compiler, "1.0 <= 1.0", true);
  CHECK_CONSTANT(fixture.compiler, "1.0 >= 2.0", false);
  CHECK_CONSTANT(fixture.compiler, "1.0 == 1.0", true);
  CHECK_CONSTANT(fixture.compiler, "1.0 != 2.0", true);
}

TEST_CASE("compile - function returning Float64 literal")
{
  auto const [types, tu] =
    compile_program("let f = fn(): Float64 => { return 3.14; };");
  REQUIRE(tu.functions.size() == 1);
  auto const result = basedhlir::interpret(*tu.functions[0], {});
  REQUIRE(std::holds_alternative<double>(result));
  CHECK(std::get<double>(result) == 3.14);
}

TEST_CASE("compile - function returning Float32 literal")
{
  auto const [types, tu] =
    compile_program("let f = fn(): Float32 => { return 1.5f; };");
  REQUIRE(tu.functions.size() == 1);
  auto const result = basedhlir::interpret(*tu.functions[0], {});
  REQUIRE(std::holds_alternative<float>(result));
  CHECK(std::get<float>(result) == 1.5f);
}

TEST_CASE("compile - function with Float64 parameter and arithmetic")
{
  auto const [types, tu] = compile_program(
    "let scale = fn(x: Float64): Float64 => { return x * 2.0; };"
  );
  REQUIRE(tu.functions.size() == 1);
  auto const args = std::array<basedhlir::Constant_value, 1>{double{3.5}};
  auto const result = basedhlir::interpret(*tu.functions[0], args);
  REQUIRE(std::holds_alternative<double>(result));
  CHECK(std::get<double>(result) == 7.0);
}

TEST_CASE("compile - function with Float64 comparison")
{
  auto const [types, tu] = compile_program(
    "let is_positive = fn(x: Float64): Bool => { return x > 0.0; };"
  );
  REQUIRE(tu.functions.size() == 1);
  auto const &f = *tu.functions[0];
  auto const args1 = std::array<basedhlir::Constant_value, 1>{double{1.0}};
  CHECK(std::get<bool>(basedhlir::interpret(f, args1)) == true);
  auto const args2 = std::array<basedhlir::Constant_value, 1>{double{-1.0}};
  CHECK(std::get<bool>(basedhlir::interpret(f, args2)) == false);
}

TEST_CASE("compile - return type mismatch Float32 vs Float64")
{
  CHECK_THROWS_AS(
    compile_program("let f = fn(): Float32 => { return 1.0; };"),
    basedhlir::Compilation_failure
  );
}

// --- bool operators ---

TEST_CASE("compile - bool operators")
{
  auto const [types, tu] = compile_program(
    "let f = fn(a: Int32, b: Int32): Bool => { return a == b; };"
  );
  REQUIRE(tu.functions.size() == 1);
  auto const &f = *tu.functions[0];
  auto const args1 =
    std::array<basedhlir::Constant_value, 2>{std::int32_t{1}, std::int32_t{1}};
  CHECK(std::get<bool>(basedhlir::interpret(f, args1)) == true);
  auto const args2 =
    std::array<basedhlir::Constant_value, 2>{std::int32_t{1}, std::int32_t{2}};
  CHECK(std::get<bool>(basedhlir::interpret(f, args2)) == false);
}
