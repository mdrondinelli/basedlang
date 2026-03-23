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
    CHECK(lexeme.location.line == line);
    CHECK(lexeme.location.column == column);
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
  CHECK(lexeme.location.line == 1);
  CHECK(lexeme.location.column == 1);
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
  CHECK(lexeme.location.line == 1);
  CHECK(lexeme.location.column == 1);
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
  CHECK(open.location.line == 1);
  CHECK(open.location.column == 1);
  auto const close = stream.lex();
  CHECK(close.text == "]");
  CHECK(close.token == basedlex::Token::rbracket);
  CHECK(close.location.line == 1);
  CHECK(close.location.column == 2);
}

TEST_CASE("Lexeme_stream - &mut lexes as ampersand_mut")
{
  auto ss = std::istringstream{"&mut"};
  auto binary = basedlex::Istream_binary_stream{&ss};
  auto chars = basedlex::Utf8_char_stream{&binary};
  auto stream = basedlex::Lexeme_stream{&chars};
  auto const lexeme = stream.lex();
  CHECK(lexeme.text == "&mut");
  CHECK(lexeme.token == basedlex::Token::ampersand_mut);
  CHECK(lexeme.location.line == 1);
  CHECK(lexeme.location.column == 1);
  CHECK(stream.lex().token == basedlex::Token::eof);
}

TEST_CASE("Lexeme_stream - &mut with space after lexes as ampersand_mut")
{
  auto ss = std::istringstream{"&mut x"};
  auto binary = basedlex::Istream_binary_stream{&ss};
  auto chars = basedlex::Utf8_char_stream{&binary};
  auto stream = basedlex::Lexeme_stream{&chars};
  auto const amp_mut = stream.lex();
  CHECK(amp_mut.text == "&mut");
  CHECK(amp_mut.token == basedlex::Token::ampersand_mut);
  auto const id = stream.lex();
  CHECK(id.text == "x");
  CHECK(id.token == basedlex::Token::identifier);
}

TEST_CASE("Lexeme_stream - &mutable lexes as ampersand + identifier")
{
  auto ss = std::istringstream{"&mutable"};
  auto binary = basedlex::Istream_binary_stream{&ss};
  auto chars = basedlex::Utf8_char_stream{&binary};
  auto stream = basedlex::Lexeme_stream{&chars};
  auto const amp = stream.lex();
  CHECK(amp.text == "&");
  CHECK(amp.token == basedlex::Token::ampersand);
  auto const id = stream.lex();
  CHECK(id.text == "mutable");
  CHECK(id.token == basedlex::Token::identifier);
}

TEST_CASE("Lexeme_stream - &mut_ lexes as ampersand + identifier")
{
  auto ss = std::istringstream{"&mut_"};
  auto binary = basedlex::Istream_binary_stream{&ss};
  auto chars = basedlex::Utf8_char_stream{&binary};
  auto stream = basedlex::Lexeme_stream{&chars};
  auto const amp = stream.lex();
  CHECK(amp.text == "&");
  CHECK(amp.token == basedlex::Token::ampersand);
  auto const id = stream.lex();
  CHECK(id.text == "mut_");
  CHECK(id.token == basedlex::Token::identifier);
}

TEST_CASE("Lexeme_stream - &mut2 lexes as ampersand + identifier")
{
  auto ss = std::istringstream{"&mut2"};
  auto binary = basedlex::Istream_binary_stream{&ss};
  auto chars = basedlex::Utf8_char_stream{&binary};
  auto stream = basedlex::Lexeme_stream{&chars};
  auto const amp = stream.lex();
  CHECK(amp.text == "&");
  CHECK(amp.token == basedlex::Token::ampersand);
  auto const id = stream.lex();
  CHECK(id.text == "mut2");
  CHECK(id.token == basedlex::Token::identifier);
}

TEST_CASE("Lexeme_stream - & alone lexes as ampersand")
{
  auto ss = std::istringstream{"&"};
  auto binary = basedlex::Istream_binary_stream{&ss};
  auto chars = basedlex::Utf8_char_stream{&binary};
  auto stream = basedlex::Lexeme_stream{&chars};
  auto const lexeme = stream.lex();
  CHECK(lexeme.text == "&");
  CHECK(lexeme.token == basedlex::Token::ampersand);
  CHECK(stream.lex().token == basedlex::Token::eof);
}

TEST_CASE("Lexeme_stream - & x lexes as ampersand + identifier")
{
  auto ss = std::istringstream{"& x"};
  auto binary = basedlex::Istream_binary_stream{&ss};
  auto chars = basedlex::Utf8_char_stream{&binary};
  auto stream = basedlex::Lexeme_stream{&chars};
  auto const amp = stream.lex();
  CHECK(amp.text == "&");
  CHECK(amp.token == basedlex::Token::ampersand);
  auto const id = stream.lex();
  CHECK(id.text == "x");
  CHECK(id.token == basedlex::Token::identifier);
}

TEST_CASE("Lexeme_stream - &mut column tracking")
{
  auto ss = std::istringstream{"x &mut y"};
  auto binary = basedlex::Istream_binary_stream{&ss};
  auto chars = basedlex::Utf8_char_stream{&binary};
  auto stream = basedlex::Lexeme_stream{&chars};
  auto const x = stream.lex();
  CHECK(x.location.column == 1);
  auto const amp_mut = stream.lex();
  CHECK(amp_mut.text == "&mut");
  CHECK(amp_mut.token == basedlex::Token::ampersand_mut);
  CHECK(amp_mut.location.column == 3);
  auto const y = stream.lex();
  CHECK(y.text == "y");
  CHECK(y.location.column == 8);
}

TEST_CASE("Lexeme_stream - &mu lexes as ampersand + identifier")
{
  auto ss = std::istringstream{"&mu"};
  auto binary = basedlex::Istream_binary_stream{&ss};
  auto chars = basedlex::Utf8_char_stream{&binary};
  auto stream = basedlex::Lexeme_stream{&chars};
  auto const amp = stream.lex();
  CHECK(amp.text == "&");
  CHECK(amp.token == basedlex::Token::ampersand);
  auto const id = stream.lex();
  CHECK(id.text == "mu");
  CHECK(id.token == basedlex::Token::identifier);
}
