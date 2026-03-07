#include <fstream>
#include <memory>

#include <catch2/catch_test_macros.hpp>

#include "basedlex/lexeme_stream.h"
#include "basedlex/lexeme_stream_reader.h"
#include "basedlex/utf8_char_stream.h"

#include "basedparse/parser.h"
#include "basedparse/statement.h"

#include "basedlex/istream_binary_stream.h"

TEST_CASE("Parser - first.based produces a Function_definition")
{
  auto file = std::ifstream{std::string{EXAMPLES_PATH} + "/first.based"};
  auto binary_stream = basedlex::Istream_binary_stream{&file};
  auto char_stream = basedlex::Utf8_char_stream{&binary_stream};
  auto lexeme_stream = basedlex::Lexeme_stream{&char_stream};
  auto reader = basedlex::Lexeme_stream_reader{&lexeme_stream};
  auto parser = basedparse::Parser{&reader};
  auto const unit = parser.parse_translation_unit();
  REQUIRE(unit.statements.size() == 1);
  auto const *stmt = unit.statements[0].get();
  auto const *fn_def =
    dynamic_cast<basedparse::Function_definition const *>(stmt);
  REQUIRE(fn_def != nullptr);
  CHECK(fn_def->kw_let.text == "let");
  CHECK(fn_def->name.text == "main");
  CHECK(fn_def->eq.text == "=");
  CHECK(fn_def->function.kw_fn.text == "fn");
  CHECK(fn_def->function.lparen.text == "(");
  CHECK(fn_def->function.rparen.text == ")");
  CHECK(fn_def->function.arrow.text == "->");
  auto const *return_type =
    dynamic_cast<basedparse::Identifier_type_expression const *>(
      fn_def->function.return_type.get()
    );
  REQUIRE(return_type != nullptr);
  CHECK(return_type->identifier.text == "i32");
  REQUIRE(fn_def->function.body != nullptr);
  CHECK(fn_def->function.body->lbrace.text == "{");
  REQUIRE(fn_def->function.body->statements.size() == 1);
  auto const *ret_stmt = dynamic_cast<basedparse::Return_statement const *>(
    fn_def->function.body->statements[0].get()
  );
  REQUIRE(ret_stmt != nullptr);
  CHECK(ret_stmt->kw_return.text == "return");
  auto const *int_lit =
    dynamic_cast<basedparse::Int_literal_expression const *>(
      ret_stmt->value.get()
    );
  REQUIRE(int_lit != nullptr);
  CHECK(int_lit->literal.text == "0");
  CHECK(ret_stmt->semicolon.text == ";");
  CHECK(fn_def->function.body->rbrace.text == "}");
}
