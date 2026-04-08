#include <fstream>
#include <sstream>

#include <catch2/catch_test_macros.hpp>

#include "kalelex/istream_binary_stream.h"
#include "kalelex/lexeme.h"
#include "kalelex/lexeme_stream.h"
#include "kalelex/token.h"
#include "kalelex/utf8_char_stream.h"

TEST_CASE("Lexeme_stream lexes first.kale")
{
  auto file = std::ifstream{EXAMPLES_PATH "/first.kale"};
  REQUIRE(file.is_open());
  auto binary = kalelex::Istream_binary_stream{&file};
  auto chars = kalelex::Utf8_char_stream{&binary};
  auto stream = kalelex::Lexeme_stream{&chars};
  auto const expect =
    [&](std::string const &text, kalelex::Token token, int line, int column)
  {
    auto const lexeme = stream.lex();
    CHECK(lexeme.text == text);
    CHECK(lexeme.token == token);
    CHECK(lexeme.location.line == line);
    CHECK(lexeme.location.column == column);
  };
  using enum kalelex::Token;
  expect("let", kw_let, 1, 1);
  expect("main", identifier, 1, 5);
  expect("=", eq, 1, 10);
  expect("fn", kw_fn, 1, 12);
  expect("(", lparen, 1, 14);
  expect(")", rparen, 1, 15);
  expect(":", colon, 1, 16);
  expect("Int32", identifier, 1, 18);
  expect("=>", fat_arrow, 1, 24);
  expect("{", lbrace, 1, 27);
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
  auto binary = kalelex::Istream_binary_stream{&ss};
  auto chars = kalelex::Utf8_char_stream{&binary};
  auto stream = kalelex::Lexeme_stream{&chars};
  using enum kalelex::Token;
  auto const expect = [&](std::string const &text, kalelex::Token token)
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
  auto binary = kalelex::Istream_binary_stream{&ss};
  auto chars = kalelex::Utf8_char_stream{&binary};
  auto stream = kalelex::Lexeme_stream{&chars};
  auto const lexeme = stream.lex();
  CHECK(lexeme.text == "->");
  CHECK(lexeme.token == kalelex::Token::arrow);
}

TEST_CASE("Lexeme_stream - minus adjacent to digits lexes as separate tokens")
{
  auto ss = std::istringstream{"1-2"};
  auto binary = kalelex::Istream_binary_stream{&ss};
  auto chars = kalelex::Utf8_char_stream{&binary};
  auto stream = kalelex::Lexeme_stream{&chars};
  using enum kalelex::Token;
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

TEST_CASE("Lexeme_stream lexes i8 integer literal")
{
  auto ss = std::istringstream{"42i8"};
  auto binary = kalelex::Istream_binary_stream{&ss};
  auto chars = kalelex::Utf8_char_stream{&binary};
  auto stream = kalelex::Lexeme_stream{&chars};
  auto const lexeme = stream.lex();
  CHECK(lexeme.text == "42i8");
  CHECK(lexeme.token == kalelex::Token::int_literal);
}

TEST_CASE("Lexeme_stream lexes i16 integer literal")
{
  auto ss = std::istringstream{"100i16"};
  auto binary = kalelex::Istream_binary_stream{&ss};
  auto chars = kalelex::Utf8_char_stream{&binary};
  auto stream = kalelex::Lexeme_stream{&chars};
  auto const lexeme = stream.lex();
  CHECK(lexeme.text == "100i16");
  CHECK(lexeme.token == kalelex::Token::int_literal);
}

TEST_CASE("Lexeme_stream lexes i32 integer literal")
{
  auto ss = std::istringstream{"100i32"};
  auto binary = kalelex::Istream_binary_stream{&ss};
  auto chars = kalelex::Utf8_char_stream{&binary};
  auto stream = kalelex::Lexeme_stream{&chars};
  auto const lexeme = stream.lex();
  CHECK(lexeme.text == "100i32");
  CHECK(lexeme.token == kalelex::Token::int_literal);
}

TEST_CASE("Lexeme_stream lexes i64 integer literal")
{
  auto ss = std::istringstream{"100i64"};
  auto binary = kalelex::Istream_binary_stream{&ss};
  auto chars = kalelex::Utf8_char_stream{&binary};
  auto stream = kalelex::Lexeme_stream{&chars};
  auto const lexeme = stream.lex();
  CHECK(lexeme.text == "100i64");
  CHECK(lexeme.token == kalelex::Token::int_literal);
}

TEST_CASE("Lexeme_stream throws on bare i suffix")
{
  auto ss = std::istringstream{"42i "};
  auto binary = kalelex::Istream_binary_stream{&ss};
  auto chars = kalelex::Utf8_char_stream{&binary};
  auto stream = kalelex::Lexeme_stream{&chars};
  CHECK_THROWS_AS(stream.lex(), kalelex::Lexeme_stream::Lex_error);
}

TEST_CASE("Lexeme_stream lexes unknown integer suffix as token")
{
  auto ss = std::istringstream{"42i7"};
  auto binary = kalelex::Istream_binary_stream{&ss};
  auto chars = kalelex::Utf8_char_stream{&binary};
  auto stream = kalelex::Lexeme_stream{&chars};
  auto const lexeme = stream.lex();
  CHECK(lexeme.text == "42i7");
  CHECK(lexeme.token == kalelex::Token::int_literal);
}

TEST_CASE("Lexeme_stream lexes colon")
{
  auto ss = std::istringstream{":"};
  auto binary = kalelex::Istream_binary_stream{&ss};
  auto chars = kalelex::Utf8_char_stream{&binary};
  auto stream = kalelex::Lexeme_stream{&chars};
  auto const lexeme = stream.lex();
  CHECK(lexeme.text == ":");
  CHECK(lexeme.token == kalelex::Token::colon);
  CHECK(lexeme.location.line == 1);
  CHECK(lexeme.location.column == 1);
}

TEST_CASE("Lexeme_stream lexes comma")
{
  auto ss = std::istringstream{","};
  auto binary = kalelex::Istream_binary_stream{&ss};
  auto chars = kalelex::Utf8_char_stream{&binary};
  auto stream = kalelex::Lexeme_stream{&chars};
  auto const lexeme = stream.lex();
  CHECK(lexeme.text == ",");
  CHECK(lexeme.token == kalelex::Token::comma);
  CHECK(lexeme.location.line == 1);
  CHECK(lexeme.location.column == 1);
}

TEST_CASE("Lexeme_stream lexes brackets")
{
  auto ss = std::istringstream{"[]"};
  auto binary = kalelex::Istream_binary_stream{&ss};
  auto chars = kalelex::Utf8_char_stream{&binary};
  auto stream = kalelex::Lexeme_stream{&chars};
  auto const open = stream.lex();
  CHECK(open.text == "[");
  CHECK(open.token == kalelex::Token::lbracket);
  CHECK(open.location.line == 1);
  CHECK(open.location.column == 1);
  auto const close = stream.lex();
  CHECK(close.text == "]");
  CHECK(close.token == kalelex::Token::rbracket);
  CHECK(close.location.line == 1);
  CHECK(close.location.column == 2);
}

TEST_CASE("Lexeme_stream - &mut lexes as ampersand_mut")
{
  auto ss = std::istringstream{"&mut"};
  auto binary = kalelex::Istream_binary_stream{&ss};
  auto chars = kalelex::Utf8_char_stream{&binary};
  auto stream = kalelex::Lexeme_stream{&chars};
  auto const lexeme = stream.lex();
  CHECK(lexeme.text == "&mut");
  CHECK(lexeme.token == kalelex::Token::ampersand_mut);
  CHECK(lexeme.location.line == 1);
  CHECK(lexeme.location.column == 1);
  CHECK(stream.lex().token == kalelex::Token::eof);
}

TEST_CASE("Lexeme_stream - &mut with space after lexes as ampersand_mut")
{
  auto ss = std::istringstream{"&mut x"};
  auto binary = kalelex::Istream_binary_stream{&ss};
  auto chars = kalelex::Utf8_char_stream{&binary};
  auto stream = kalelex::Lexeme_stream{&chars};
  auto const amp_mut = stream.lex();
  CHECK(amp_mut.text == "&mut");
  CHECK(amp_mut.token == kalelex::Token::ampersand_mut);
  auto const id = stream.lex();
  CHECK(id.text == "x");
  CHECK(id.token == kalelex::Token::identifier);
}

TEST_CASE("Lexeme_stream - &mutable lexes as ampersand + identifier")
{
  auto ss = std::istringstream{"&mutable"};
  auto binary = kalelex::Istream_binary_stream{&ss};
  auto chars = kalelex::Utf8_char_stream{&binary};
  auto stream = kalelex::Lexeme_stream{&chars};
  auto const amp = stream.lex();
  CHECK(amp.text == "&");
  CHECK(amp.token == kalelex::Token::ampersand);
  auto const id = stream.lex();
  CHECK(id.text == "mutable");
  CHECK(id.token == kalelex::Token::identifier);
}

TEST_CASE("Lexeme_stream - &mut_ lexes as ampersand + identifier")
{
  auto ss = std::istringstream{"&mut_"};
  auto binary = kalelex::Istream_binary_stream{&ss};
  auto chars = kalelex::Utf8_char_stream{&binary};
  auto stream = kalelex::Lexeme_stream{&chars};
  auto const amp = stream.lex();
  CHECK(amp.text == "&");
  CHECK(amp.token == kalelex::Token::ampersand);
  auto const id = stream.lex();
  CHECK(id.text == "mut_");
  CHECK(id.token == kalelex::Token::identifier);
}

TEST_CASE("Lexeme_stream - &mut2 lexes as ampersand + identifier")
{
  auto ss = std::istringstream{"&mut2"};
  auto binary = kalelex::Istream_binary_stream{&ss};
  auto chars = kalelex::Utf8_char_stream{&binary};
  auto stream = kalelex::Lexeme_stream{&chars};
  auto const amp = stream.lex();
  CHECK(amp.text == "&");
  CHECK(amp.token == kalelex::Token::ampersand);
  auto const id = stream.lex();
  CHECK(id.text == "mut2");
  CHECK(id.token == kalelex::Token::identifier);
}

TEST_CASE("Lexeme_stream - & alone lexes as ampersand")
{
  auto ss = std::istringstream{"&"};
  auto binary = kalelex::Istream_binary_stream{&ss};
  auto chars = kalelex::Utf8_char_stream{&binary};
  auto stream = kalelex::Lexeme_stream{&chars};
  auto const lexeme = stream.lex();
  CHECK(lexeme.text == "&");
  CHECK(lexeme.token == kalelex::Token::ampersand);
  CHECK(stream.lex().token == kalelex::Token::eof);
}

TEST_CASE("Lexeme_stream - & x lexes as ampersand + identifier")
{
  auto ss = std::istringstream{"& x"};
  auto binary = kalelex::Istream_binary_stream{&ss};
  auto chars = kalelex::Utf8_char_stream{&binary};
  auto stream = kalelex::Lexeme_stream{&chars};
  auto const amp = stream.lex();
  CHECK(amp.text == "&");
  CHECK(amp.token == kalelex::Token::ampersand);
  auto const id = stream.lex();
  CHECK(id.text == "x");
  CHECK(id.token == kalelex::Token::identifier);
}

TEST_CASE("Lexeme_stream - &mut column tracking")
{
  auto ss = std::istringstream{"x &mut y"};
  auto binary = kalelex::Istream_binary_stream{&ss};
  auto chars = kalelex::Utf8_char_stream{&binary};
  auto stream = kalelex::Lexeme_stream{&chars};
  auto const x = stream.lex();
  CHECK(x.location.column == 1);
  auto const amp_mut = stream.lex();
  CHECK(amp_mut.text == "&mut");
  CHECK(amp_mut.token == kalelex::Token::ampersand_mut);
  CHECK(amp_mut.location.column == 3);
  auto const y = stream.lex();
  CHECK(y.text == "y");
  CHECK(y.location.column == 8);
}

TEST_CASE("Lexeme_stream - &mu lexes as ampersand + identifier")
{
  auto ss = std::istringstream{"&mu"};
  auto binary = kalelex::Istream_binary_stream{&ss};
  auto chars = kalelex::Utf8_char_stream{&binary};
  auto stream = kalelex::Lexeme_stream{&chars};
  auto const amp = stream.lex();
  CHECK(amp.text == "&");
  CHECK(amp.token == kalelex::Token::ampersand);
  auto const id = stream.lex();
  CHECK(id.text == "mu");
  CHECK(id.token == kalelex::Token::identifier);
}

TEST_CASE("Lexeme_stream - ^mut lexes as caret_mut")
{
  auto ss = std::istringstream{"^mut"};
  auto binary = kalelex::Istream_binary_stream{&ss};
  auto chars = kalelex::Utf8_char_stream{&binary};
  auto stream = kalelex::Lexeme_stream{&chars};
  auto const lexeme = stream.lex();
  CHECK(lexeme.text == "^mut");
  CHECK(lexeme.token == kalelex::Token::caret_mut);
  CHECK(lexeme.location.line == 1);
  CHECK(lexeme.location.column == 1);
  CHECK(stream.lex().token == kalelex::Token::eof);
}

TEST_CASE("Lexeme_stream - ^mut with space after lexes as caret_mut")
{
  auto ss = std::istringstream{"^mut x"};
  auto binary = kalelex::Istream_binary_stream{&ss};
  auto chars = kalelex::Utf8_char_stream{&binary};
  auto stream = kalelex::Lexeme_stream{&chars};
  auto const caret_mut = stream.lex();
  CHECK(caret_mut.text == "^mut");
  CHECK(caret_mut.token == kalelex::Token::caret_mut);
  auto const id = stream.lex();
  CHECK(id.text == "x");
  CHECK(id.token == kalelex::Token::identifier);
}

TEST_CASE("Lexeme_stream - ^mutable lexes as caret + identifier")
{
  auto ss = std::istringstream{"^mutable"};
  auto binary = kalelex::Istream_binary_stream{&ss};
  auto chars = kalelex::Utf8_char_stream{&binary};
  auto stream = kalelex::Lexeme_stream{&chars};
  auto const caret = stream.lex();
  CHECK(caret.text == "^");
  CHECK(caret.token == kalelex::Token::caret);
  auto const id = stream.lex();
  CHECK(id.text == "mutable");
  CHECK(id.token == kalelex::Token::identifier);
}

TEST_CASE("Lexeme_stream - ^mut_ lexes as caret + identifier")
{
  auto ss = std::istringstream{"^mut_"};
  auto binary = kalelex::Istream_binary_stream{&ss};
  auto chars = kalelex::Utf8_char_stream{&binary};
  auto stream = kalelex::Lexeme_stream{&chars};
  auto const caret = stream.lex();
  CHECK(caret.text == "^");
  CHECK(caret.token == kalelex::Token::caret);
  auto const id = stream.lex();
  CHECK(id.text == "mut_");
  CHECK(id.token == kalelex::Token::identifier);
}

TEST_CASE("Lexeme_stream - ^ alone lexes as caret")
{
  auto ss = std::istringstream{"^"};
  auto binary = kalelex::Istream_binary_stream{&ss};
  auto chars = kalelex::Utf8_char_stream{&binary};
  auto stream = kalelex::Lexeme_stream{&chars};
  auto const lexeme = stream.lex();
  CHECK(lexeme.text == "^");
  CHECK(lexeme.token == kalelex::Token::caret);
  CHECK(stream.lex().token == kalelex::Token::eof);
}

TEST_CASE("Lexeme_stream - ^mut column tracking")
{
  auto ss = std::istringstream{"x ^mut y"};
  auto binary = kalelex::Istream_binary_stream{&ss};
  auto chars = kalelex::Utf8_char_stream{&binary};
  auto stream = kalelex::Lexeme_stream{&chars};
  auto const x = stream.lex();
  CHECK(x.location.column == 1);
  auto const caret_mut = stream.lex();
  CHECK(caret_mut.text == "^mut");
  CHECK(caret_mut.token == kalelex::Token::caret_mut);
  CHECK(caret_mut.location.column == 3);
  auto const y = stream.lex();
  CHECK(y.text == "y");
  CHECK(y.location.column == 8);
}

TEST_CASE("Lexeme_stream lexes unsuffixed float literal")
{
  auto ss = std::istringstream{"3.14"};
  auto binary = kalelex::Istream_binary_stream{&ss};
  auto chars = kalelex::Utf8_char_stream{&binary};
  auto stream = kalelex::Lexeme_stream{&chars};
  auto const lexeme = stream.lex();
  CHECK(lexeme.text == "3.14");
  CHECK(lexeme.token == kalelex::Token::float_literal);
}

TEST_CASE("Lexeme_stream lexes f-suffixed float literal")
{
  auto ss = std::istringstream{"1.5f"};
  auto binary = kalelex::Istream_binary_stream{&ss};
  auto chars = kalelex::Utf8_char_stream{&binary};
  auto stream = kalelex::Lexeme_stream{&chars};
  auto const lexeme = stream.lex();
  CHECK(lexeme.text == "1.5f");
  CHECK(lexeme.token == kalelex::Token::float_literal);
}

TEST_CASE("Lexeme_stream lexes d-suffixed float literal")
{
  auto ss = std::istringstream{"1.5d"};
  auto binary = kalelex::Istream_binary_stream{&ss};
  auto chars = kalelex::Utf8_char_stream{&binary};
  auto stream = kalelex::Lexeme_stream{&chars};
  auto const lexeme = stream.lex();
  CHECK(lexeme.text == "1.5d");
  CHECK(lexeme.token == kalelex::Token::float_literal);
}

TEST_CASE("Lexeme_stream lexes integer with f suffix as float literal")
{
  auto ss = std::istringstream{"1f"};
  auto binary = kalelex::Istream_binary_stream{&ss};
  auto chars = kalelex::Utf8_char_stream{&binary};
  auto stream = kalelex::Lexeme_stream{&chars};
  auto const lexeme = stream.lex();
  CHECK(lexeme.text == "1f");
  CHECK(lexeme.token == kalelex::Token::float_literal);
}

TEST_CASE("Lexeme_stream lexes integer with d suffix as float literal")
{
  auto ss = std::istringstream{"1d"};
  auto binary = kalelex::Istream_binary_stream{&ss};
  auto chars = kalelex::Utf8_char_stream{&binary};
  auto stream = kalelex::Lexeme_stream{&chars};
  auto const lexeme = stream.lex();
  CHECK(lexeme.text == "1d");
  CHECK(lexeme.token == kalelex::Token::float_literal);
}

TEST_CASE("Lexeme_stream - integer with space before dot stays int")
{
  // "1 ." — the 1 is an int_literal; the dot is not consumed by the int lex
  auto ss = std::istringstream{"1 ."};
  auto binary = kalelex::Istream_binary_stream{&ss};
  auto chars = kalelex::Utf8_char_stream{&binary};
  auto stream = kalelex::Lexeme_stream{&chars};
  auto const first = stream.lex();
  CHECK(first.text == "1");
  CHECK(first.token == kalelex::Token::int_literal);
}

TEST_CASE("Lexeme_stream - integer with trailing dot lexes as float literal")
{
  auto ss = std::istringstream{"1."};
  auto binary = kalelex::Istream_binary_stream{&ss};
  auto chars = kalelex::Utf8_char_stream{&binary};
  auto stream = kalelex::Lexeme_stream{&chars};
  auto const lexeme = stream.lex();
  CHECK(lexeme.text == "1.");
  CHECK(lexeme.token == kalelex::Token::float_literal);
}

TEST_CASE("Lexeme_stream lexes f suffix without trailing digits as float literal")
{
  auto ss = std::istringstream{"3.14f "};
  auto binary = kalelex::Istream_binary_stream{&ss};
  auto chars = kalelex::Utf8_char_stream{&binary};
  auto stream = kalelex::Lexeme_stream{&chars};
  auto const lexeme = stream.lex();
  CHECK(lexeme.text == "3.14f");
  CHECK(lexeme.token == kalelex::Token::float_literal);
}
