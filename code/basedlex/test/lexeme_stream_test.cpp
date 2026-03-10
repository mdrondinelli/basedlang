#include <fstream>
#include <sstream>

#include <catch2/catch_test_macros.hpp>

#include "basedlex/istream_binary_stream.h"
#include "basedlex/lexeme.h"
#include "basedlex/lexeme_stream.h"
#include "basedlex/token.h"
#include "basedlex/utf8_char_stream.h"

TEST_CASE("Lexeme_stream lexes first.based")
{
  auto file = std::ifstream{EXAMPLES_PATH "/first.based"};
  REQUIRE(file.is_open());
  auto binary = basedlex::Istream_binary_stream{&file};
  auto chars = basedlex::Utf8_char_stream{&binary};
  auto stream = basedlex::Lexeme_stream{&chars};
  auto const expect =
    [&](std::string const &text, basedlex::Token token, int line, int column)
  {
    auto const lexeme = stream.lex();
    CHECK(lexeme.text == text);
    CHECK(lexeme.token == token);
    CHECK(lexeme.line == line);
    CHECK(lexeme.column == column);
  };
  using enum basedlex::Token;
  expect("let", kw_let, 1, 1);
  expect("main", identifier, 1, 5);
  expect("=", eq, 1, 10);
  expect("fn", kw_fn, 1, 12);
  expect("(", lparen, 1, 14);
  expect(")", rparen, 1, 15);
  expect(":", colon, 1, 16);
  expect("i32", identifier, 1, 18);
  expect("->", arrow, 1, 22);
  expect("{", lbrace, 1, 25);
  expect("return", kw_return, 2, 3);
  expect("0", int_literal, 2, 10);
  expect(";", semicolon, 2, 11);
  expect("}", rbrace, 3, 1);
  expect(";", semicolon, 3, 2);
  expect("", eof, 4, 1);
}

TEST_CASE("Lexeme_stream lexes arithmetic operators")
{
  auto ss = std::istringstream{"+ - * / %"};
  auto binary = basedlex::Istream_binary_stream{&ss};
  auto chars = basedlex::Utf8_char_stream{&binary};
  auto stream = basedlex::Lexeme_stream{&chars};
  using enum basedlex::Token;
  auto const expect = [&](std::string const &text, basedlex::Token token)
  {
    auto const lexeme = stream.lex();
    CHECK(lexeme.text == text);
    CHECK(lexeme.token == token);
  };
  expect("+", plus);
  expect("-", minus);
  expect("*", star);
  expect("/", slash);
  expect("%", percent);
}

TEST_CASE("Lexeme_stream - minus before arrow still lexes as arrow")
{
  auto ss = std::istringstream{"->"};
  auto binary = basedlex::Istream_binary_stream{&ss};
  auto chars = basedlex::Utf8_char_stream{&binary};
  auto stream = basedlex::Lexeme_stream{&chars};
  auto const lexeme = stream.lex();
  CHECK(lexeme.text == "->");
  CHECK(lexeme.token == basedlex::Token::arrow);
}

TEST_CASE("Lexeme_stream - minus adjacent to digits lexes as separate tokens")
{
  auto ss = std::istringstream{"1-2"};
  auto binary = basedlex::Istream_binary_stream{&ss};
  auto chars = basedlex::Utf8_char_stream{&binary};
  auto stream = basedlex::Lexeme_stream{&chars};
  using enum basedlex::Token;
  auto const first = stream.lex();
  CHECK(first.text == "1");
  CHECK(first.token == int_literal);
  auto const op = stream.lex();
  CHECK(op.text == "-");
  CHECK(op.token == minus);
  auto const second = stream.lex();
  CHECK(second.text == "2");
  CHECK(second.token == int_literal);
}

TEST_CASE("Lexeme_stream lexes colon")
{
  auto ss = std::istringstream{":"};
  auto binary = basedlex::Istream_binary_stream{&ss};
  auto chars = basedlex::Utf8_char_stream{&binary};
  auto stream = basedlex::Lexeme_stream{&chars};
  auto const lexeme = stream.lex();
  CHECK(lexeme.text == ":");
  CHECK(lexeme.token == basedlex::Token::colon);
  CHECK(lexeme.line == 1);
  CHECK(lexeme.column == 1);
}

TEST_CASE("Lexeme_stream lexes comma")
{
  auto ss = std::istringstream{","};
  auto binary = basedlex::Istream_binary_stream{&ss};
  auto chars = basedlex::Utf8_char_stream{&binary};
  auto stream = basedlex::Lexeme_stream{&chars};
  auto const lexeme = stream.lex();
  CHECK(lexeme.text == ",");
  CHECK(lexeme.token == basedlex::Token::comma);
  CHECK(lexeme.line == 1);
  CHECK(lexeme.column == 1);
}

TEST_CASE("Lexeme_stream lexes brackets")
{
  auto ss = std::istringstream{"[]"};
  auto binary = basedlex::Istream_binary_stream{&ss};
  auto chars = basedlex::Utf8_char_stream{&binary};
  auto stream = basedlex::Lexeme_stream{&chars};
  auto const open = stream.lex();
  CHECK(open.text == "[");
  CHECK(open.token == basedlex::Token::lbracket);
  CHECK(open.line == 1);
  CHECK(open.column == 1);
  auto const close = stream.lex();
  CHECK(close.text == "]");
  CHECK(close.token == basedlex::Token::rbracket);
  CHECK(close.line == 1);
  CHECK(close.column == 2);
}
