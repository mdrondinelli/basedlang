#include <fstream>
#include <sstream>
#include <string_view>

#include <catch2/catch_test_macros.hpp>

#include "lexing/lexeme.h"
#include "lexing/lexeme_stream.h"
#include "lexing/token.h"
#include "spelling/spelling.h"
#include "streams/istream_binary_stream.h"
#include "streams/utf8_char_stream.h"

namespace
{

  void check_span(
    benson::Lexeme const &lexeme,
    int start_line,
    int start_column,
    int end_line,
    int end_column
  )
  {
    CHECK(lexeme.span.start.line == start_line);
    CHECK(lexeme.span.start.column == start_column);
    CHECK(lexeme.span.end.line == end_line);
    CHECK(lexeme.span.end.column == end_column);
    CHECK(benson::span_of(lexeme).start.line == start_line);
    CHECK(benson::span_of(lexeme).start.column == start_column);
    CHECK(benson::span_of(lexeme).end.line == end_line);
    CHECK(benson::span_of(lexeme).end.column == end_column);
  }

} // namespace

TEST_CASE("Lexeme_stream lexes first.benson")
{
  auto file = std::ifstream{EXAMPLES_PATH "/first.benson"};
  REQUIRE(file.is_open());
  auto binary = benson::Istream_binary_stream{&file};
  auto chars = benson::Utf8_char_stream{&binary};
  auto spellings = benson::Spelling_table{};
  auto stream = benson::Lexeme_stream{&chars, &spellings};
  auto const expect = [&](
                        std::string const &text,
                        benson::Token token,
                        int line,
                        int column,
                        int end_column
                      )
  {
    auto const lexeme = stream.lex();
    CHECK(spellings.lookup(lexeme.spelling) == text);
    CHECK(lexeme.token == token);
    check_span(lexeme, line, column, line, end_column);
  };
  using enum benson::Token;
  expect("fn", kw_fn, 1, 1, 2);
  expect("main", identifier, 1, 4, 7);
  expect("(", lparen, 1, 8, 8);
  expect(")", rparen, 1, 9, 9);
  expect(":", colon, 1, 10, 10);
  expect("Int32", identifier, 1, 12, 16);
  expect("=>", fat_arrow, 1, 18, 19);
  expect("{", lbrace, 1, 21, 21);
  expect("return", kw_return, 2, 3, 8);
  expect("0", int_literal, 2, 10, 10);
  expect(";", semicolon, 2, 11, 11);
  expect("}", rbrace, 3, 1, 1);
  expect(";", semicolon, 3, 2, 2);
  expect("", eof, 4, 1, 1);
}

TEST_CASE("Lexeme_stream lexes arithmetic operators")
{
  auto ss = std::istringstream{"+ - * / %"};
  auto binary = benson::Istream_binary_stream{&ss};
  auto chars = benson::Utf8_char_stream{&binary};
  auto spellings = benson::Spelling_table{};
  auto stream = benson::Lexeme_stream{&chars, &spellings};
  using enum benson::Token;
  auto const expect = [&](std::string const &text, benson::Token token)
  {
    auto const lexeme = stream.lex();
    CHECK(spellings.lookup(lexeme.spelling) == text);
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
  auto binary = benson::Istream_binary_stream{&ss};
  auto chars = benson::Utf8_char_stream{&binary};
  auto spellings = benson::Spelling_table{};
  auto stream = benson::Lexeme_stream{&chars, &spellings};
  auto const lexeme = stream.lex();
  CHECK(spellings.lookup(lexeme.spelling) == "->");
  CHECK(lexeme.token == benson::Token::arrow);
  check_span(lexeme, 1, 1, 1, 2);
}

TEST_CASE("Lexeme_stream - minus adjacent to digits lexes as separate tokens")
{
  auto ss = std::istringstream{"1-2"};
  auto binary = benson::Istream_binary_stream{&ss};
  auto chars = benson::Utf8_char_stream{&binary};
  auto spellings = benson::Spelling_table{};
  auto stream = benson::Lexeme_stream{&chars, &spellings};
  using enum benson::Token;
  auto const first = stream.lex();
  CHECK(spellings.lookup(first.spelling) == "1");
  CHECK(first.token == int_literal);
  check_span(first, 1, 1, 1, 1);
  auto const op = stream.lex();
  CHECK(spellings.lookup(op.spelling) == "-");
  CHECK(op.token == minus);
  check_span(op, 1, 2, 1, 2);
  auto const second = stream.lex();
  CHECK(spellings.lookup(second.spelling) == "2");
  CHECK(second.token == int_literal);
  check_span(second, 1, 3, 1, 3);
}

TEST_CASE("Lexeme_stream lexes i8 integer literal")
{
  auto ss = std::istringstream{"42i8"};
  auto binary = benson::Istream_binary_stream{&ss};
  auto chars = benson::Utf8_char_stream{&binary};
  auto spellings = benson::Spelling_table{};
  auto stream = benson::Lexeme_stream{&chars, &spellings};
  auto const lexeme = stream.lex();
  CHECK(spellings.lookup(lexeme.spelling) == "42i8");
  CHECK(lexeme.token == benson::Token::int_literal);
  check_span(lexeme, 1, 1, 1, 4);
}

TEST_CASE("Lexeme_stream lexes i16 integer literal")
{
  auto ss = std::istringstream{"100i16"};
  auto binary = benson::Istream_binary_stream{&ss};
  auto chars = benson::Utf8_char_stream{&binary};
  auto spellings = benson::Spelling_table{};
  auto stream = benson::Lexeme_stream{&chars, &spellings};
  auto const lexeme = stream.lex();
  CHECK(spellings.lookup(lexeme.spelling) == "100i16");
  CHECK(lexeme.token == benson::Token::int_literal);
  check_span(lexeme, 1, 1, 1, 6);
}

TEST_CASE("Lexeme_stream lexes i32 integer literal")
{
  auto ss = std::istringstream{"100i32"};
  auto binary = benson::Istream_binary_stream{&ss};
  auto chars = benson::Utf8_char_stream{&binary};
  auto spellings = benson::Spelling_table{};
  auto stream = benson::Lexeme_stream{&chars, &spellings};
  auto const lexeme = stream.lex();
  CHECK(spellings.lookup(lexeme.spelling) == "100i32");
  CHECK(lexeme.token == benson::Token::int_literal);
  check_span(lexeme, 1, 1, 1, 6);
}

TEST_CASE("Lexeme_stream lexes i64 integer literal")
{
  auto ss = std::istringstream{"100i64"};
  auto binary = benson::Istream_binary_stream{&ss};
  auto chars = benson::Utf8_char_stream{&binary};
  auto spellings = benson::Spelling_table{};
  auto stream = benson::Lexeme_stream{&chars, &spellings};
  auto const lexeme = stream.lex();
  CHECK(spellings.lookup(lexeme.spelling) == "100i64");
  CHECK(lexeme.token == benson::Token::int_literal);
  check_span(lexeme, 1, 1, 1, 6);
}

TEST_CASE("Lexeme_stream throws on bare i suffix")
{
  auto ss = std::istringstream{"42i "};
  auto binary = benson::Istream_binary_stream{&ss};
  auto chars = benson::Utf8_char_stream{&binary};
  auto spellings = benson::Spelling_table{};
  auto stream = benson::Lexeme_stream{&chars, &spellings};
  CHECK_THROWS_AS(stream.lex(), benson::Lexeme_stream::Lex_error);
}

TEST_CASE("Lexeme_stream lexes unknown integer suffix as token")
{
  auto ss = std::istringstream{"42i7"};
  auto binary = benson::Istream_binary_stream{&ss};
  auto chars = benson::Utf8_char_stream{&binary};
  auto spellings = benson::Spelling_table{};
  auto stream = benson::Lexeme_stream{&chars, &spellings};
  auto const lexeme = stream.lex();
  CHECK(spellings.lookup(lexeme.spelling) == "42i7");
  CHECK(lexeme.token == benson::Token::int_literal);
  check_span(lexeme, 1, 1, 1, 4);
}

TEST_CASE("Lexeme_stream lexes colon")
{
  auto ss = std::istringstream{":"};
  auto binary = benson::Istream_binary_stream{&ss};
  auto chars = benson::Utf8_char_stream{&binary};
  auto spellings = benson::Spelling_table{};
  auto stream = benson::Lexeme_stream{&chars, &spellings};
  auto const lexeme = stream.lex();
  CHECK(spellings.lookup(lexeme.spelling) == ":");
  CHECK(lexeme.token == benson::Token::colon);
  check_span(lexeme, 1, 1, 1, 1);
}

TEST_CASE("Lexeme_stream lexes comma")
{
  auto ss = std::istringstream{","};
  auto binary = benson::Istream_binary_stream{&ss};
  auto chars = benson::Utf8_char_stream{&binary};
  auto spellings = benson::Spelling_table{};
  auto stream = benson::Lexeme_stream{&chars, &spellings};
  auto const lexeme = stream.lex();
  CHECK(spellings.lookup(lexeme.spelling) == ",");
  CHECK(lexeme.token == benson::Token::comma);
  check_span(lexeme, 1, 1, 1, 1);
}

TEST_CASE("Lexeme_stream lexes brackets")
{
  auto ss = std::istringstream{"[]"};
  auto binary = benson::Istream_binary_stream{&ss};
  auto chars = benson::Utf8_char_stream{&binary};
  auto spellings = benson::Spelling_table{};
  auto stream = benson::Lexeme_stream{&chars, &spellings};
  auto const open = stream.lex();
  CHECK(spellings.lookup(open.spelling) == "[");
  CHECK(open.token == benson::Token::lbracket);
  check_span(open, 1, 1, 1, 1);
  auto const close = stream.lex();
  CHECK(spellings.lookup(close.spelling) == "]");
  CHECK(close.token == benson::Token::rbracket);
  check_span(close, 1, 2, 1, 2);
}

TEST_CASE("Lexeme_stream - &mut lexes as ampersand_mut")
{
  auto ss = std::istringstream{"&mut"};
  auto binary = benson::Istream_binary_stream{&ss};
  auto chars = benson::Utf8_char_stream{&binary};
  auto spellings = benson::Spelling_table{};
  auto stream = benson::Lexeme_stream{&chars, &spellings};
  auto const lexeme = stream.lex();
  CHECK(spellings.lookup(lexeme.spelling) == "&mut");
  CHECK(lexeme.token == benson::Token::ampersand_mut);
  check_span(lexeme, 1, 1, 1, 4);
  check_span(stream.lex(), 1, 5, 1, 5);
}

TEST_CASE("Lexeme_stream - &mut with space after lexes as ampersand_mut")
{
  auto ss = std::istringstream{"&mut x"};
  auto binary = benson::Istream_binary_stream{&ss};
  auto chars = benson::Utf8_char_stream{&binary};
  auto spellings = benson::Spelling_table{};
  auto stream = benson::Lexeme_stream{&chars, &spellings};
  auto const amp_mut = stream.lex();
  CHECK(spellings.lookup(amp_mut.spelling) == "&mut");
  CHECK(amp_mut.token == benson::Token::ampersand_mut);
  auto const id = stream.lex();
  CHECK(spellings.lookup(id.spelling) == "x");
  CHECK(id.token == benson::Token::identifier);
}

TEST_CASE("Lexeme_stream - &mutable lexes as ampersand + identifier")
{
  auto ss = std::istringstream{"&mutable"};
  auto binary = benson::Istream_binary_stream{&ss};
  auto chars = benson::Utf8_char_stream{&binary};
  auto spellings = benson::Spelling_table{};
  auto stream = benson::Lexeme_stream{&chars, &spellings};
  auto const amp = stream.lex();
  CHECK(spellings.lookup(amp.spelling) == "&");
  CHECK(amp.token == benson::Token::ampersand);
  auto const id = stream.lex();
  CHECK(spellings.lookup(id.spelling) == "mutable");
  CHECK(id.token == benson::Token::identifier);
}

TEST_CASE("Lexeme_stream - &mut_ lexes as ampersand + identifier")
{
  auto ss = std::istringstream{"&mut_"};
  auto binary = benson::Istream_binary_stream{&ss};
  auto chars = benson::Utf8_char_stream{&binary};
  auto spellings = benson::Spelling_table{};
  auto stream = benson::Lexeme_stream{&chars, &spellings};
  auto const amp = stream.lex();
  CHECK(spellings.lookup(amp.spelling) == "&");
  CHECK(amp.token == benson::Token::ampersand);
  auto const id = stream.lex();
  CHECK(spellings.lookup(id.spelling) == "mut_");
  CHECK(id.token == benson::Token::identifier);
}

TEST_CASE("Lexeme_stream - &mut2 lexes as ampersand + identifier")
{
  auto ss = std::istringstream{"&mut2"};
  auto binary = benson::Istream_binary_stream{&ss};
  auto chars = benson::Utf8_char_stream{&binary};
  auto spellings = benson::Spelling_table{};
  auto stream = benson::Lexeme_stream{&chars, &spellings};
  auto const amp = stream.lex();
  CHECK(spellings.lookup(amp.spelling) == "&");
  CHECK(amp.token == benson::Token::ampersand);
  auto const id = stream.lex();
  CHECK(spellings.lookup(id.spelling) == "mut2");
  CHECK(id.token == benson::Token::identifier);
}

TEST_CASE("Lexeme_stream - & alone lexes as ampersand")
{
  auto ss = std::istringstream{"&"};
  auto binary = benson::Istream_binary_stream{&ss};
  auto chars = benson::Utf8_char_stream{&binary};
  auto spellings = benson::Spelling_table{};
  auto stream = benson::Lexeme_stream{&chars, &spellings};
  auto const lexeme = stream.lex();
  CHECK(spellings.lookup(lexeme.spelling) == "&");
  CHECK(lexeme.token == benson::Token::ampersand);
  CHECK(stream.lex().token == benson::Token::eof);
}

TEST_CASE("Lexeme_stream - & x lexes as ampersand + identifier")
{
  auto ss = std::istringstream{"& x"};
  auto binary = benson::Istream_binary_stream{&ss};
  auto chars = benson::Utf8_char_stream{&binary};
  auto spellings = benson::Spelling_table{};
  auto stream = benson::Lexeme_stream{&chars, &spellings};
  auto const amp = stream.lex();
  CHECK(spellings.lookup(amp.spelling) == "&");
  CHECK(amp.token == benson::Token::ampersand);
  auto const id = stream.lex();
  CHECK(spellings.lookup(id.spelling) == "x");
  CHECK(id.token == benson::Token::identifier);
}

TEST_CASE("Lexeme_stream - &mut column tracking")
{
  auto ss = std::istringstream{"x &mut y"};
  auto binary = benson::Istream_binary_stream{&ss};
  auto chars = benson::Utf8_char_stream{&binary};
  auto spellings = benson::Spelling_table{};
  auto stream = benson::Lexeme_stream{&chars, &spellings};
  auto const x = stream.lex();
  CHECK(x.span.start.column == 1);
  CHECK(x.span.end.column == 1);
  auto const amp_mut = stream.lex();
  CHECK(spellings.lookup(amp_mut.spelling) == "&mut");
  CHECK(amp_mut.token == benson::Token::ampersand_mut);
  CHECK(amp_mut.span.start.column == 3);
  CHECK(amp_mut.span.end.column == 6);
  auto const y = stream.lex();
  CHECK(spellings.lookup(y.spelling) == "y");
  CHECK(y.span.start.column == 8);
  CHECK(y.span.end.column == 8);
}

TEST_CASE("Lexeme_stream - &mu lexes as ampersand + identifier")
{
  auto ss = std::istringstream{"&mu"};
  auto binary = benson::Istream_binary_stream{&ss};
  auto chars = benson::Utf8_char_stream{&binary};
  auto spellings = benson::Spelling_table{};
  auto stream = benson::Lexeme_stream{&chars, &spellings};
  auto const amp = stream.lex();
  CHECK(spellings.lookup(amp.spelling) == "&");
  CHECK(amp.token == benson::Token::ampersand);
  auto const id = stream.lex();
  CHECK(spellings.lookup(id.spelling) == "mu");
  CHECK(id.token == benson::Token::identifier);
}

TEST_CASE("Lexeme_stream - ^mut lexes as caret_mut")
{
  auto ss = std::istringstream{"^mut"};
  auto binary = benson::Istream_binary_stream{&ss};
  auto chars = benson::Utf8_char_stream{&binary};
  auto spellings = benson::Spelling_table{};
  auto stream = benson::Lexeme_stream{&chars, &spellings};
  auto const lexeme = stream.lex();
  CHECK(spellings.lookup(lexeme.spelling) == "^mut");
  CHECK(lexeme.token == benson::Token::caret_mut);
  check_span(lexeme, 1, 1, 1, 4);
  check_span(stream.lex(), 1, 5, 1, 5);
}

TEST_CASE("Lexeme_stream - ^mut with space after lexes as caret_mut")
{
  auto ss = std::istringstream{"^mut x"};
  auto binary = benson::Istream_binary_stream{&ss};
  auto chars = benson::Utf8_char_stream{&binary};
  auto spellings = benson::Spelling_table{};
  auto stream = benson::Lexeme_stream{&chars, &spellings};
  auto const caret_mut = stream.lex();
  CHECK(spellings.lookup(caret_mut.spelling) == "^mut");
  CHECK(caret_mut.token == benson::Token::caret_mut);
  auto const id = stream.lex();
  CHECK(spellings.lookup(id.spelling) == "x");
  CHECK(id.token == benson::Token::identifier);
}

TEST_CASE("Lexeme_stream - ^mutable lexes as caret + identifier")
{
  auto ss = std::istringstream{"^mutable"};
  auto binary = benson::Istream_binary_stream{&ss};
  auto chars = benson::Utf8_char_stream{&binary};
  auto spellings = benson::Spelling_table{};
  auto stream = benson::Lexeme_stream{&chars, &spellings};
  auto const caret = stream.lex();
  CHECK(spellings.lookup(caret.spelling) == "^");
  CHECK(caret.token == benson::Token::caret);
  auto const id = stream.lex();
  CHECK(spellings.lookup(id.spelling) == "mutable");
  CHECK(id.token == benson::Token::identifier);
}

TEST_CASE("Lexeme_stream - ^mut_ lexes as caret + identifier")
{
  auto ss = std::istringstream{"^mut_"};
  auto binary = benson::Istream_binary_stream{&ss};
  auto chars = benson::Utf8_char_stream{&binary};
  auto spellings = benson::Spelling_table{};
  auto stream = benson::Lexeme_stream{&chars, &spellings};
  auto const caret = stream.lex();
  CHECK(spellings.lookup(caret.spelling) == "^");
  CHECK(caret.token == benson::Token::caret);
  auto const id = stream.lex();
  CHECK(spellings.lookup(id.spelling) == "mut_");
  CHECK(id.token == benson::Token::identifier);
}

TEST_CASE("Lexeme_stream - ^ alone lexes as caret")
{
  auto ss = std::istringstream{"^"};
  auto binary = benson::Istream_binary_stream{&ss};
  auto chars = benson::Utf8_char_stream{&binary};
  auto spellings = benson::Spelling_table{};
  auto stream = benson::Lexeme_stream{&chars, &spellings};
  auto const lexeme = stream.lex();
  CHECK(spellings.lookup(lexeme.spelling) == "^");
  CHECK(lexeme.token == benson::Token::caret);
  CHECK(stream.lex().token == benson::Token::eof);
}

TEST_CASE("Lexeme_stream - ^mut column tracking")
{
  auto ss = std::istringstream{"x ^mut y"};
  auto binary = benson::Istream_binary_stream{&ss};
  auto chars = benson::Utf8_char_stream{&binary};
  auto spellings = benson::Spelling_table{};
  auto stream = benson::Lexeme_stream{&chars, &spellings};
  auto const x = stream.lex();
  CHECK(x.span.start.column == 1);
  CHECK(x.span.end.column == 1);
  auto const caret_mut = stream.lex();
  CHECK(spellings.lookup(caret_mut.spelling) == "^mut");
  CHECK(caret_mut.token == benson::Token::caret_mut);
  CHECK(caret_mut.span.start.column == 3);
  CHECK(caret_mut.span.end.column == 6);
  auto const y = stream.lex();
  CHECK(spellings.lookup(y.spelling) == "y");
  CHECK(y.span.start.column == 8);
  CHECK(y.span.end.column == 8);
}

TEST_CASE("Lexeme_stream lexes unsuffixed float literal")
{
  auto ss = std::istringstream{"3.14"};
  auto binary = benson::Istream_binary_stream{&ss};
  auto chars = benson::Utf8_char_stream{&binary};
  auto spellings = benson::Spelling_table{};
  auto stream = benson::Lexeme_stream{&chars, &spellings};
  auto const lexeme = stream.lex();
  CHECK(spellings.lookup(lexeme.spelling) == "3.14");
  CHECK(lexeme.token == benson::Token::float_literal);
  check_span(lexeme, 1, 1, 1, 4);
}

TEST_CASE("Lexeme_stream lexes f-suffixed float literal")
{
  auto ss = std::istringstream{"1.5f"};
  auto binary = benson::Istream_binary_stream{&ss};
  auto chars = benson::Utf8_char_stream{&binary};
  auto spellings = benson::Spelling_table{};
  auto stream = benson::Lexeme_stream{&chars, &spellings};
  auto const lexeme = stream.lex();
  CHECK(spellings.lookup(lexeme.spelling) == "1.5f");
  CHECK(lexeme.token == benson::Token::float_literal);
  check_span(lexeme, 1, 1, 1, 4);
}

TEST_CASE("Lexeme_stream lexes d-suffixed float literal")
{
  auto ss = std::istringstream{"1.5d"};
  auto binary = benson::Istream_binary_stream{&ss};
  auto chars = benson::Utf8_char_stream{&binary};
  auto spellings = benson::Spelling_table{};
  auto stream = benson::Lexeme_stream{&chars, &spellings};
  auto const lexeme = stream.lex();
  CHECK(spellings.lookup(lexeme.spelling) == "1.5d");
  CHECK(lexeme.token == benson::Token::float_literal);
  check_span(lexeme, 1, 1, 1, 4);
}

TEST_CASE("Lexeme_stream lexes integer with f suffix as float literal")
{
  auto ss = std::istringstream{"1f"};
  auto binary = benson::Istream_binary_stream{&ss};
  auto chars = benson::Utf8_char_stream{&binary};
  auto spellings = benson::Spelling_table{};
  auto stream = benson::Lexeme_stream{&chars, &spellings};
  auto const lexeme = stream.lex();
  CHECK(spellings.lookup(lexeme.spelling) == "1f");
  CHECK(lexeme.token == benson::Token::float_literal);
  check_span(lexeme, 1, 1, 1, 2);
}

TEST_CASE("Lexeme_stream lexes integer with d suffix as float literal")
{
  auto ss = std::istringstream{"1d"};
  auto binary = benson::Istream_binary_stream{&ss};
  auto chars = benson::Utf8_char_stream{&binary};
  auto spellings = benson::Spelling_table{};
  auto stream = benson::Lexeme_stream{&chars, &spellings};
  auto const lexeme = stream.lex();
  CHECK(spellings.lookup(lexeme.spelling) == "1d");
  CHECK(lexeme.token == benson::Token::float_literal);
  check_span(lexeme, 1, 1, 1, 2);
}

TEST_CASE("Lexeme_stream - integer with space before dot stays int")
{
  // "1 ." — the 1 is an int_literal; the dot is not consumed by the int lex
  auto ss = std::istringstream{"1 ."};
  auto binary = benson::Istream_binary_stream{&ss};
  auto chars = benson::Utf8_char_stream{&binary};
  auto spellings = benson::Spelling_table{};
  auto stream = benson::Lexeme_stream{&chars, &spellings};
  auto const first = stream.lex();
  CHECK(spellings.lookup(first.spelling) == "1");
  CHECK(first.token == benson::Token::int_literal);
  check_span(first, 1, 1, 1, 1);
}

TEST_CASE("Lexeme_stream - integer with trailing dot lexes as float literal")
{
  auto ss = std::istringstream{"1."};
  auto binary = benson::Istream_binary_stream{&ss};
  auto chars = benson::Utf8_char_stream{&binary};
  auto spellings = benson::Spelling_table{};
  auto stream = benson::Lexeme_stream{&chars, &spellings};
  auto const lexeme = stream.lex();
  CHECK(spellings.lookup(lexeme.spelling) == "1.");
  CHECK(lexeme.token == benson::Token::float_literal);
  check_span(lexeme, 1, 1, 1, 2);
}

TEST_CASE(
  "Lexeme_stream lexes f suffix without trailing digits as float literal"
)
{
  auto ss = std::istringstream{"3.14f "};
  auto binary = benson::Istream_binary_stream{&ss};
  auto chars = benson::Utf8_char_stream{&binary};
  auto spellings = benson::Spelling_table{};
  auto stream = benson::Lexeme_stream{&chars, &spellings};
  auto const lexeme = stream.lex();
  CHECK(spellings.lookup(lexeme.spelling) == "3.14f");
  CHECK(lexeme.token == benson::Token::float_literal);
  check_span(lexeme, 1, 1, 1, 5);
}

TEST_CASE("Lexeme_stream assigns spelling to every token")
{
  auto ss = std::istringstream{"let x = 42;"};
  auto binary = benson::Istream_binary_stream{&ss};
  auto chars = benson::Utf8_char_stream{&binary};
  auto spellings = benson::Spelling_table{};
  auto stream = benson::Lexeme_stream{&chars, &spellings};

  for (;;)
  {
    auto const lexeme = stream.lex();
    CHECK(lexeme.spelling.has_value());
    if (lexeme.token == benson::Token::eof)
    {
      CHECK(spellings.lookup(lexeme.spelling).empty());
      break;
    }
  }
}

TEST_CASE("Lexeme_stream deduplicates repeated spellings")
{
  auto ss = std::istringstream{"x x + +"};
  auto binary = benson::Istream_binary_stream{&ss};
  auto chars = benson::Utf8_char_stream{&binary};
  auto spellings = benson::Spelling_table{};
  auto stream = benson::Lexeme_stream{&chars, &spellings};

  auto const first_identifier = stream.lex();
  auto const second_identifier = stream.lex();
  auto const first_plus = stream.lex();
  auto const second_plus = stream.lex();

  CHECK(first_identifier.spelling == second_identifier.spelling);
  CHECK(first_plus.spelling == second_plus.spelling);
}
