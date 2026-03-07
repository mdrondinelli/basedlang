#include <fstream>

#include <catch2/catch_test_macros.hpp>

#include "basedlex/lexeme.h"
#include "basedlex/lexer.h"
#include "basedlex/token.h"
#include "basedlex/utf8_char_stream.h"
#include "istream_binary_stream.h"

TEST_CASE("Lexer lexes first.based")
{
  auto file = std::ifstream{EXAMPLES_PATH "/first.based"};
  REQUIRE(file.is_open());
  auto binary = basedlex::Istream_binary_stream{&file};
  auto chars = basedlex::Utf8_char_stream{&binary};
  auto lexer = basedlex::Lexer{&chars};
  auto const expect = [&](std::string const &text, basedlex::Token token, int line, int column)
  {
    auto const result = lexer.lex();
    REQUIRE(std::holds_alternative<basedlex::Lexeme>(result));
    auto const &lexeme = std::get<basedlex::Lexeme>(result);
    CHECK(lexeme.text == text);
    CHECK(lexeme.token == token);
    CHECK(lexeme.line == line);
    CHECK(lexeme.column == column);
  };
  using enum basedlex::Token;
  expect("let",    kw_let,      1,  1);
  expect("main",   identifier,  1,  5);
  expect("=",      eq,          1, 10);
  expect("fn",     kw_fn,       1, 12);
  expect("(",      lparen,      1, 14);
  expect(")",      rparen,      1, 15);
  expect("->",     arrow,       1, 17);
  expect("i32",    identifier,  1, 20);
  expect("{",      lbrace,      1, 24);
  expect("return", kw_return,   2,  3);
  expect("0",      int_literal, 2, 10);
  expect(";",      semicolon,   2, 11);
  expect("}",      rbrace,      3,  1);
  expect("",       eof,         4,  1);
}
