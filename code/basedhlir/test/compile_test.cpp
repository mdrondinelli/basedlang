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

std::unique_ptr<Parse_fixture> make_parse_fixture(std::string source)
{
  return std::make_unique<Parse_fixture>(std::move(source));
}

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

TEST_CASE("parse_int_literal")
{
  auto const value = basedhlir::parse_int_literal("42");
  REQUIRE(value.has_value());
  CHECK(*value == 42);
}

TEST_CASE("parse_int_literal - uint64 overflow")
{
  CHECK_FALSE(basedhlir::parse_int_literal("18446744073709551616").has_value());
}

TEST_CASE("evaluate_constant_expression - integer literal")
{
  auto parse_fixture = Parse_fixture{"1"};
  auto compile_fixture = Compile_fixture{};
  auto const expr = parse_fixture.parser.parse_expression();

  auto const value =
    compile_fixture.compiler.evaluate_constant_expression(*expr);
  REQUIRE(std::holds_alternative<int32_t>(value));
  CHECK(std::get<int32_t>(value) == 1);
}

TEST_CASE("evaluate_constant_expression - integer literal above int32 max")
{
  auto parse_fixture = Parse_fixture{"2147483648"};
  auto compile_fixture = Compile_fixture{};
  auto const expr = parse_fixture.parser.parse_expression();

  CHECK_THROWS_AS(
    compile_fixture.compiler.evaluate_constant_expression(*expr),
    basedhlir::Compilation_error
  );
}

TEST_CASE("evaluate_constant_expression - unary minus allows int32 minimum")
{
  auto parse_fixture = Parse_fixture{"-2147483648"};
  auto compile_fixture = Compile_fixture{};
  auto const expr = parse_fixture.parser.parse_expression();

  auto const value =
    compile_fixture.compiler.evaluate_constant_expression(*expr);
  REQUIRE(std::holds_alternative<int32_t>(value));
  CHECK(std::get<int32_t>(value) == std::numeric_limits<std::int32_t>::min());
}

TEST_CASE("evaluate_constant_expression - integer arithmetic")
{
  auto parse_fixture = Parse_fixture{"3 % 2 / (3 - 4) * 5 + 6"};
  auto compile_fixture = Compile_fixture{};
  auto const expr = parse_fixture.parser.parse_expression();

  auto const value =
    compile_fixture.compiler.evaluate_constant_expression(*expr);
  REQUIRE(std::holds_alternative<int32_t>(value));
  CHECK(std::get<int32_t>(value) == 1);
}

TEST_CASE("evaluate_constant_expression - unary plus")
{
  auto parse_fixture = Parse_fixture{"+42"};
  auto compile_fixture = Compile_fixture{};
  auto const expr = parse_fixture.parser.parse_expression();

  auto const value =
    compile_fixture.compiler.evaluate_constant_expression(*expr);
  REQUIRE(std::holds_alternative<int32_t>(value));
  CHECK(std::get<int32_t>(value) == 42);
}

TEST_CASE("evaluate_constant_expression - unary minus")
{
  auto parse_fixture = Parse_fixture{"-42"};
  auto compile_fixture = Compile_fixture{};
  auto const expr = parse_fixture.parser.parse_expression();

  auto const value =
    compile_fixture.compiler.evaluate_constant_expression(*expr);
  REQUIRE(std::holds_alternative<int32_t>(value));
  CHECK(std::get<int32_t>(value) == -42);
}

TEST_CASE("evaluate_constant_expression - nested arithmetic")
{
  auto parse_fixture = Parse_fixture{"(1 + 2) * 3"};
  auto compile_fixture = Compile_fixture{};
  auto const expr = parse_fixture.parser.parse_expression();

  auto const value =
    compile_fixture.compiler.evaluate_constant_expression(*expr);
  REQUIRE(std::holds_alternative<int32_t>(value));
  CHECK(std::get<int32_t>(value) == 9);
}

TEST_CASE("evaluate_constant_expression - integer comparisons")
{
  auto compile_fixture = Compile_fixture{};
  auto cases = std::vector<std::pair<std::unique_ptr<Parse_fixture>, bool>>{};
  cases.emplace_back(make_parse_fixture("123 < 456"), true);
  cases.emplace_back(make_parse_fixture("123 > 456"), false);
  cases.emplace_back(make_parse_fixture("123 <= 456"), true);
  cases.emplace_back(make_parse_fixture("123 >= 456"), false);
  cases.emplace_back(make_parse_fixture("123 == 456"), false);
  cases.emplace_back(make_parse_fixture("123 != 456"), true);
  auto expressions = std::vector<std::unique_ptr<basedparse::Expression>>{};
  for (auto const &[parse_fixture, expected] : cases)
  {
    auto const &expr =
      *expressions.emplace_back(parse_fixture->parser.parse_expression());

    auto const value =
      compile_fixture.compiler.evaluate_constant_expression(expr);
    REQUIRE(std::holds_alternative<bool>(value));
    CHECK(std::get<bool>(value) == expected);
  }
}

TEST_CASE("evaluate_constant_expression - if expression")
{
  auto parse_fixture = Parse_fixture{"if 1 < 2 { 10 } else { 20 }"};
  auto compile_fixture = Compile_fixture{};
  auto const expr = parse_fixture.parser.parse_expression();

  auto const value =
    compile_fixture.compiler.evaluate_constant_expression(*expr);
  REQUIRE(std::holds_alternative<int32_t>(value));
  CHECK(std::get<int32_t>(value) == 10);
}

TEST_CASE("evaluate_constant_expression - if expression takes else branch")
{
  auto parse_fixture = Parse_fixture{"if 1 > 2 { 10 } else { 20 }"};
  auto compile_fixture = Compile_fixture{};
  auto const expr = parse_fixture.parser.parse_expression();

  auto const value =
    compile_fixture.compiler.evaluate_constant_expression(*expr);
  REQUIRE(std::holds_alternative<int32_t>(value));
  CHECK(std::get<int32_t>(value) == 20);
}

TEST_CASE("evaluate_constant_expression - if expression takes else-if branch")
{
  auto parse_fixture =
    Parse_fixture{"if 1 > 2 { 10 } else if 1 < 2 { 30 } else { 20 }"};
  auto compile_fixture = Compile_fixture{};
  auto const expr = parse_fixture.parser.parse_expression();

  auto const value =
    compile_fixture.compiler.evaluate_constant_expression(*expr);
  REQUIRE(std::holds_alternative<int32_t>(value));
  CHECK(std::get<int32_t>(value) == 30);
}

TEST_CASE(
  "evaluate_constant_expression - if with constant true condition folds to "
  "then branch"
)
{
  auto parse_fixture = Parse_fixture{"if 1 < 2 { 10 }"};
  auto compile_fixture = Compile_fixture{};
  auto const expr = parse_fixture.parser.parse_expression();

  auto const value =
    compile_fixture.compiler.evaluate_constant_expression(*expr);
  CHECK(std::get<std::int32_t>(value) == 10);
}

TEST_CASE(
  "evaluate_constant_expression - if with constant true condition "
  "allows different branch types"
)
{
  auto parse_fixture = Parse_fixture{"if true { 10 } else { false }"};
  auto compile_fixture = Compile_fixture{};
  auto const expr = parse_fixture.parser.parse_expression();

  auto const value =
    compile_fixture.compiler.evaluate_constant_expression(*expr);
  CHECK(std::get<std::int32_t>(value) == 10);
}

TEST_CASE("compile_type_expression - sized array")
{
  auto parse_fixture = Parse_fixture{"[4]Int32"};
  auto compile_fixture = Compile_fixture{};
  auto const expr = parse_fixture.parser.parse_expression();
  auto const type = compile_fixture.compiler.compile_type_expression(*expr);
  auto const sa = std::get_if<basedhlir::Sized_array_type>(&type->data);
  REQUIRE(sa != nullptr);
  CHECK(sa->element == compile_fixture.types.int32_type());
  CHECK(sa->size == 4);
}

TEST_CASE("compile_type_expression - sized array with constant expression size")
{
  auto parse_fixture = Parse_fixture{"[2 + 2]Int32"};
  auto compile_fixture = Compile_fixture{};
  auto const expr = parse_fixture.parser.parse_expression();
  auto const type = compile_fixture.compiler.compile_type_expression(*expr);
  auto const sa = std::get_if<basedhlir::Sized_array_type>(&type->data);
  REQUIRE(sa != nullptr);
  CHECK(sa->element == compile_fixture.types.int32_type());
  CHECK(sa->size == 4);
}

TEST_CASE("compile_type_expression - sized array rejects zero size")
{
  auto parse_fixture = Parse_fixture{"[0]Int32"};
  auto compile_fixture = Compile_fixture{};
  auto const expr = parse_fixture.parser.parse_expression();
  CHECK_THROWS_AS(
    compile_fixture.compiler.compile_type_expression(*expr),
    basedhlir::Compilation_error
  );
}

TEST_CASE("compile_type_expression - sized array rejects negative size")
{
  auto parse_fixture = Parse_fixture{"[-1]Int32"};
  auto compile_fixture = Compile_fixture{};
  auto const expr = parse_fixture.parser.parse_expression();
  CHECK_THROWS_AS(
    compile_fixture.compiler.compile_type_expression(*expr),
    basedhlir::Compilation_error
  );
}

TEST_CASE("compile_type_expression - sized array rejects non-integer size")
{
  auto parse_fixture = Parse_fixture{"[1 < 2]Int32"};
  auto compile_fixture = Compile_fixture{};
  auto const expr = parse_fixture.parser.parse_expression();
  CHECK_THROWS_AS(
    compile_fixture.compiler.compile_type_expression(*expr),
    basedhlir::Compilation_error
  );
}

TEST_CASE("evaluate_constant_expression - bool equality")
{
  auto compile_fixture = Compile_fixture{};
  auto cases = std::vector<std::pair<std::unique_ptr<Parse_fixture>, bool>>{};
  cases.emplace_back(make_parse_fixture("1 == 1 == (2 == 2)"), true);
  cases.emplace_back(make_parse_fixture("1 == 1 != (1 == 2)"), true);
  cases.emplace_back(make_parse_fixture("1 == 2 == (3 == 4)"), true);
  auto expressions = std::vector<std::unique_ptr<basedparse::Expression>>{};
  for (auto const &[parse_fixture, expected] : cases)
  {
    auto const &expr =
      *expressions.emplace_back(parse_fixture->parser.parse_expression());

    auto const value =
      compile_fixture.compiler.evaluate_constant_expression(expr);
    REQUIRE(std::holds_alternative<bool>(value));
    CHECK(std::get<bool>(value) == expected);
  }
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
