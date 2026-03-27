#include <catch2/catch_test_macros.hpp>

#include "basedlex/istream_binary_stream.h"
#include "basedlex/lexeme_stream.h"
#include "basedlex/lexeme_stream_reader.h"
#include "basedlex/utf8_char_stream.h"

#include "basedparse/parser.h"

#include "basedhlir/compile.h"
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

TEST_CASE("evaluate_constant_expression - integer literal")
{
  auto parse_fixture = Parse_fixture{"1"};
  auto compile_fixture = Compile_fixture{};
  auto const expr = parse_fixture.parser.parse_expression();
  REQUIRE(compile_fixture.compiler.is_constant_expression(*expr));
  auto const value =
    compile_fixture.compiler.evaluate_constant_expression(*expr);
  REQUIRE(std::holds_alternative<int32_t>(value));
  CHECK(std::get<int32_t>(value) == 1);
}

TEST_CASE("evaluate_constant_expression - integer arithmetic")
{
  auto parse_fixture = Parse_fixture{"3 % 2 / (3 - 4) * 5 + 6"};
  auto compile_fixture = Compile_fixture{};
  auto const expr = parse_fixture.parser.parse_expression();
  REQUIRE(compile_fixture.compiler.is_constant_expression(*expr));
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
  REQUIRE(compile_fixture.compiler.is_constant_expression(*expr));
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
  REQUIRE(compile_fixture.compiler.is_constant_expression(*expr));
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
  REQUIRE(compile_fixture.compiler.is_constant_expression(*expr));
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
    REQUIRE(compile_fixture.compiler.is_constant_expression(expr));
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
  REQUIRE(compile_fixture.compiler.is_constant_expression(*expr));
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
  REQUIRE(compile_fixture.compiler.is_constant_expression(*expr));
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
  REQUIRE(compile_fixture.compiler.is_constant_expression(*expr));
  auto const value =
    compile_fixture.compiler.evaluate_constant_expression(*expr);
  REQUIRE(std::holds_alternative<int32_t>(value));
  CHECK(std::get<int32_t>(value) == 30);
}

TEST_CASE("evaluate_constant_expression - if without else is void")
{
  auto parse_fixture = Parse_fixture{"if 1 < 2 { 10 }"};
  auto compile_fixture = Compile_fixture{};
  auto const expr = parse_fixture.parser.parse_expression();
  REQUIRE(compile_fixture.compiler.is_constant_expression(*expr));
  auto const value =
    compile_fixture.compiler.evaluate_constant_expression(*expr);
  CHECK(std::holds_alternative<basedhlir::Void_value>(value));
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
    REQUIRE(compile_fixture.compiler.is_constant_expression(expr));
    auto const value =
      compile_fixture.compiler.evaluate_constant_expression(expr);
    REQUIRE(std::holds_alternative<bool>(value));
    CHECK(std::get<bool>(value) == expected);
  }
}
