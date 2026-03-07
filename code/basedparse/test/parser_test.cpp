#include <fstream>
#include <memory>
#include <sstream>
#include <string>

#include <catch2/catch_test_macros.hpp>

#include "basedlex/istream_binary_stream.h"
#include "basedlex/lexeme_stream.h"
#include "basedlex/lexeme_stream_reader.h"
#include "basedlex/utf8_char_stream.h"

#include "basedparse/parser.h"
#include "basedparse/statement.h"

struct Parse_fixture
{
  std::istringstream stream;
  basedlex::Istream_binary_stream binary_stream;
  basedlex::Utf8_char_stream char_stream;
  basedlex::Lexeme_stream lexeme_stream;
  basedlex::Lexeme_stream_reader reader;
  basedparse::Parser parser;

  explicit Parse_fixture(std::string const &source)
      : stream{source},
        binary_stream{&stream},
        char_stream{&binary_stream},
        lexeme_stream{&char_stream},
        reader{&lexeme_stream},
        parser{&reader}
  {
  }
};

static bool parses(std::string const &source)
{
  auto fixture = Parse_fixture{source};
  try
  {
    fixture.parser.parse_translation_unit();
    return true;
  }
  catch (...)
  {
    return false;
  }
}

TEST_CASE("Parser - first.based produces a Function_definition")
{
  auto file = std::ifstream{std::string{EXAMPLES_PATH} + "/first.based"};
  auto binary_stream = basedlex::Istream_binary_stream{&file};
  auto char_stream = basedlex::Utf8_char_stream{&binary_stream};
  auto lexeme_stream = basedlex::Lexeme_stream{&char_stream};
  auto reader = basedlex::Lexeme_stream_reader{&lexeme_stream};
  auto parser = basedparse::Parser{&reader};
  auto const unit = parser.parse_translation_unit();
  REQUIRE(unit->statements.size() == 1);
  auto const *stmt = unit->statements[0].get();
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

TEST_CASE("Parser - accepts valid code")
{
  CHECK(parses(""));
  CHECK(parses("let main = fn() -> i32 { return 0; }"));
  CHECK(parses("let main = fn() -> i32 { }"));
  CHECK(parses(
    "let main = fn() -> i32 { return 0; }\n"
    "let other = fn() -> i32 { return 1; }"
  ));
  CHECK(parses("let main = fn() -> void { x; }"));
  CHECK(parses("let main = fn() -> void { let x = 42; }"));
}

TEST_CASE("Parser - rejects invalid code")
{
  CHECK_FALSE(parses("let"));
  CHECK_FALSE(parses("let x"));
  CHECK_FALSE(parses("let x ="));
  CHECK_FALSE(parses("let x = 42;"));
  CHECK_FALSE(parses("let x = fn()"));
  CHECK_FALSE(parses("let x = fn() ->"));
  CHECK_FALSE(parses("let x = fn() -> i32"));
  CHECK_FALSE(parses("let x = fn() -> i32 {"));
  CHECK_FALSE(parses("return 0;"));
  CHECK_FALSE(parses("{"));
  CHECK_FALSE(parses("42"));
  CHECK_FALSE(parses("let = fn() -> i32 { }"));
  CHECK_FALSE(parses("let x = fn() -> i32 { return; }"));
}

TEST_CASE("parse_translation_unit - empty")
{
  auto fixture = Parse_fixture{""};
  auto const unit = fixture.parser.parse_translation_unit();
  CHECK(unit->statements.empty());
}

TEST_CASE("parse_translation_unit - multiple functions")
{
  auto fixture = Parse_fixture{
    "let a = fn() -> i32 { return 1; }\n"
    "let b = fn() -> i32 { return 2; }"
  };
  auto const unit = fixture.parser.parse_translation_unit();
  REQUIRE(unit->statements.size() == 2);
  auto const *a = dynamic_cast<basedparse::Function_definition const *>(
    unit->statements[0].get()
  );
  auto const *b = dynamic_cast<basedparse::Function_definition const *>(
    unit->statements[1].get()
  );
  REQUIRE(a != nullptr);
  REQUIRE(b != nullptr);
  CHECK(a->name.text == "a");
  CHECK(b->name.text == "b");
}

TEST_CASE("parse_function_definition")
{
  auto fixture = Parse_fixture{"let main = fn() -> i32 { return 0; }"};
  auto const fn_def = fixture.parser.parse_function_definition();
  CHECK(fn_def->kw_let.text == "let");
  CHECK(fn_def->name.text == "main");
  CHECK(fn_def->eq.text == "=");
  CHECK(fn_def->function.kw_fn.text == "fn");
  REQUIRE(fn_def->function.body != nullptr);
  CHECK(fn_def->function.body->statements.size() == 1);
}

TEST_CASE("parse_let_statement")
{
  auto fixture = Parse_fixture{"let x = 42;"};
  auto const stmt = fixture.parser.parse_let_statement();
  CHECK(stmt->kw_let.text == "let");
  CHECK(stmt->name.text == "x");
  CHECK(stmt->eq.text == "=");
  auto const *lit = dynamic_cast<basedparse::Int_literal_expression const *>(
    stmt->initializer.get()
  );
  REQUIRE(lit != nullptr);
  CHECK(lit->literal.text == "42");
  CHECK(stmt->semicolon.text == ";");
}

TEST_CASE("parse_return_statement")
{
  auto fixture = Parse_fixture{"return 99;"};
  auto const stmt = fixture.parser.parse_return_statement();
  CHECK(stmt->kw_return.text == "return");
  auto const *lit =
    dynamic_cast<basedparse::Int_literal_expression const *>(stmt->value.get());
  REQUIRE(lit != nullptr);
  CHECK(lit->literal.text == "99");
  CHECK(stmt->semicolon.text == ";");
}

TEST_CASE("parse_expression_statement")
{
  auto fixture = Parse_fixture{"foo;"};
  auto const stmt = fixture.parser.parse_expression_statement();
  auto const *id = dynamic_cast<basedparse::Identifier_expression const *>(
    stmt->expression.get()
  );
  REQUIRE(id != nullptr);
  CHECK(id->identifier.text == "foo");
  CHECK(stmt->semicolon.text == ";");
}

TEST_CASE("parse_block_statement")
{
  auto fixture = Parse_fixture{"{ return 1; let x = 2; }"};
  auto const block = fixture.parser.parse_block_statement();
  CHECK(block->lbrace.text == "{");
  REQUIRE(block->statements.size() == 2);
  CHECK(
    dynamic_cast<basedparse::Return_statement const *>(
      block->statements[0].get()
    ) != nullptr
  );
  CHECK(
    dynamic_cast<basedparse::Let_statement const *>(
      block->statements[1].get()
    ) != nullptr
  );
  CHECK(block->rbrace.text == "}");
}

TEST_CASE("parse_block_statement - empty")
{
  auto fixture = Parse_fixture{"{ }"};
  auto const block = fixture.parser.parse_block_statement();
  CHECK(block->lbrace.text == "{");
  CHECK(block->statements.empty());
  CHECK(block->rbrace.text == "}");
}

TEST_CASE("parse_fn_expression")
{
  auto fixture = Parse_fixture{"fn() -> i32 { return 0; }"};
  auto const fn = fixture.parser.parse_fn_expression();
  CHECK(fn->kw_fn.text == "fn");
  CHECK(fn->lparen.text == "(");
  CHECK(fn->rparen.text == ")");
  CHECK(fn->arrow.text == "->");
  auto const *ret_type =
    dynamic_cast<basedparse::Identifier_type_expression const *>(
      fn->return_type.get()
    );
  REQUIRE(ret_type != nullptr);
  CHECK(ret_type->identifier.text == "i32");
  REQUIRE(fn->body != nullptr);
  CHECK(fn->body->statements.size() == 1);
}

TEST_CASE("parse_int_literal_expression")
{
  auto fixture = Parse_fixture{"42"};
  auto const expr = fixture.parser.parse_int_literal_expression();
  CHECK(expr->literal.text == "42");
}

TEST_CASE("parse_identifier_expression")
{
  auto fixture = Parse_fixture{"foo"};
  auto const expr = fixture.parser.parse_identifier_expression();
  CHECK(expr->identifier.text == "foo");
}

TEST_CASE("parse_identifier_type_expression")
{
  auto fixture = Parse_fixture{"i32"};
  auto const expr = fixture.parser.parse_identifier_type_expression();
  CHECK(expr->identifier.text == "i32");
}

TEST_CASE("parse_expression - dispatches to int literal")
{
  auto fixture = Parse_fixture{"123"};
  auto const expr = fixture.parser.parse_expression();
  CHECK(
    dynamic_cast<basedparse::Int_literal_expression const *>(expr.get()) !=
    nullptr
  );
}

TEST_CASE("parse_expression - dispatches to identifier")
{
  auto fixture = Parse_fixture{"x"};
  auto const expr = fixture.parser.parse_expression();
  CHECK(
    dynamic_cast<basedparse::Identifier_expression const *>(expr.get()) !=
    nullptr
  );
}

TEST_CASE("parse_expression - dispatches to fn")
{
  auto fixture = Parse_fixture{"fn() -> i32 { }"};
  auto const expr = fixture.parser.parse_expression();
  CHECK(dynamic_cast<basedparse::Fn_expression const *>(expr.get()) != nullptr);
}

TEST_CASE("parse_type_expression - dispatches to identifier type")
{
  auto fixture = Parse_fixture{"void"};
  auto const expr = fixture.parser.parse_type_expression();
  CHECK(
    dynamic_cast<basedparse::Identifier_type_expression const *>(expr.get()) !=
    nullptr
  );
}

TEST_CASE("parse_statement - dispatches to let")
{
  auto fixture = Parse_fixture{"let x = 1;"};
  auto const stmt = fixture.parser.parse_statement();
  CHECK(dynamic_cast<basedparse::Let_statement const *>(stmt.get()) != nullptr);
}

TEST_CASE("parse_statement - dispatches to return")
{
  auto fixture = Parse_fixture{"return 1;"};
  auto const stmt = fixture.parser.parse_statement();
  CHECK(
    dynamic_cast<basedparse::Return_statement const *>(stmt.get()) != nullptr
  );
}

TEST_CASE("parse_statement - dispatches to expression statement")
{
  auto fixture = Parse_fixture{"x;"};
  auto const stmt = fixture.parser.parse_statement();
  CHECK(
    dynamic_cast<basedparse::Expression_statement const *>(stmt.get()) !=
    nullptr
  );
}
