#include <fstream>
#include <memory>
#include <sstream>
#include <string>
#include <string_view>

#include <catch2/catch_test_macros.hpp>

#include "lexing/lexeme_stream.h"
#include "lexing/lexeme_stream_reader.h"

#include "parsing/parser.h"
#include "spelling/spelling.h"
#include "streams/istream_binary_stream.h"
#include "streams/utf8_char_stream.h"

struct Parse_fixture
{
  std::istringstream stream;
  benson::Istream_binary_stream binary_stream;
  benson::Utf8_char_stream char_stream;
  benson::Spelling_table spellings;
  benson::Lexeme_stream lexeme_stream;
  benson::Lexeme_stream_reader reader;
  benson::Parser parser;

  explicit Parse_fixture(std::string const &source)
      : stream{source},
        binary_stream{&stream},
        char_stream{&binary_stream},
        spellings{},
        lexeme_stream{&char_stream, &spellings},
        reader{&lexeme_stream},
        parser{&reader, &spellings}
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

static benson::ast::Fn_expression const &
top_level_fn(benson::ast::Statement const &statement)
{
  auto const expr_stmt =
    std::get_if<benson::ast::Expression_statement>(&statement.value);
  REQUIRE(expr_stmt != nullptr);
  auto const fn =
    std::get_if<benson::ast::Fn_expression>(&expr_stmt->expression.value);
  REQUIRE(fn != nullptr);
  return *fn;
}

TEST_CASE("Parser - first.benson produces a declaration")
{
  auto file = std::ifstream{std::string{EXAMPLES_PATH} + "/first.benson"};
  auto binary_stream = benson::Istream_binary_stream{&file};
  auto char_stream = benson::Utf8_char_stream{&binary_stream};
  auto spellings = benson::Spelling_table{};
  auto lexeme_stream = benson::Lexeme_stream{&char_stream, &spellings};
  auto reader = benson::Lexeme_stream_reader{&lexeme_stream};
  auto parser = benson::Parser{&reader, &spellings};
  auto const unit = parser.parse_translation_unit();
  REQUIRE(unit.statements.size() == 1);
  auto const &fn = top_level_fn(unit.statements[0]);
  CHECK(spellings.lookup(fn.kw_fn.spelling) == "fn");
  REQUIRE(fn.name.has_value());
  CHECK(spellings.lookup(fn.name->spelling) == "main");
  CHECK(spellings.lookup(fn.lparen.spelling) == "(");
  CHECK(spellings.lookup(fn.rparen.spelling) == ")");
  CHECK(spellings.lookup(fn.return_type_specifier.colon.spelling) == ":");
  auto const return_type = std::get_if<benson::ast::Identifier_expression>(
    &fn.return_type_specifier.type->value
  );
  REQUIRE(return_type != nullptr);
  CHECK(spellings.lookup(return_type->identifier.spelling) == "Int32");
  CHECK(spellings.lookup(fn.arrow.spelling) == "=>");
  REQUIRE(fn.body != nullptr);
  auto const body = std::get_if<benson::ast::Block_expression>(&fn.body->value);
  REQUIRE(body != nullptr);
  CHECK(spellings.lookup(body->lbrace.spelling) == "{");
  REQUIRE(body->statements.size() == 1);
  auto const ret_stmt =
    std::get_if<benson::ast::Return_statement>(&body->statements[0].value);
  REQUIRE(ret_stmt != nullptr);
  CHECK(spellings.lookup(ret_stmt->kw_return.spelling) == "return");
  auto const int_lit =
    std::get_if<benson::ast::Int_literal_expression>(&ret_stmt->value.value);
  REQUIRE(int_lit != nullptr);
  CHECK(spellings.lookup(int_lit->literal.spelling) == "0");
  CHECK(spellings.lookup(ret_stmt->semicolon.spelling) == ";");
  CHECK(spellings.lookup(body->rbrace.spelling) == "}");
}

TEST_CASE("Parser - parameters.benson parses successfully")
{
  auto file = std::ifstream{std::string{EXAMPLES_PATH} + "/parameters.benson"};
  auto binary_stream = benson::Istream_binary_stream{&file};
  auto char_stream = benson::Utf8_char_stream{&binary_stream};
  auto spellings = benson::Spelling_table{};
  auto lexeme_stream = benson::Lexeme_stream{&char_stream, &spellings};
  auto reader = benson::Lexeme_stream_reader{&lexeme_stream};
  auto parser = benson::Parser{&reader, &spellings};
  auto const unit = parser.parse_translation_unit();
  REQUIRE(unit.statements.size() == 3);
  auto const &id_fn = top_level_fn(unit.statements[0]);
  auto const &first_fn = top_level_fn(unit.statements[1]);
  auto const &main_fn = top_level_fn(unit.statements[2]);
  REQUIRE(id_fn.name.has_value());
  REQUIRE(first_fn.name.has_value());
  REQUIRE(main_fn.name.has_value());
  CHECK(spellings.lookup(id_fn.name->spelling) == "id");
  CHECK(spellings.lookup(first_fn.name->spelling) == "first");
  CHECK(spellings.lookup(main_fn.name->spelling) == "main");
  // id: fn(x: Int32): Int32 => { return x; }
  REQUIRE(id_fn.parameters.size() == 1);
  CHECK(spellings.lookup(id_fn.parameters[0].name.spelling) == "x");
  auto const id_param_type = std::get_if<benson::ast::Identifier_expression>(
    &id_fn.parameters[0].type->value
  );
  REQUIRE(id_param_type != nullptr);
  CHECK(spellings.lookup(id_param_type->identifier.spelling) == "Int32");
  auto const id_ret_type = std::get_if<benson::ast::Identifier_expression>(
    &id_fn.return_type_specifier.type->value
  );
  REQUIRE(id_ret_type != nullptr);
  CHECK(spellings.lookup(id_ret_type->identifier.spelling) == "Int32");
  auto const id_body =
    std::get_if<benson::ast::Block_expression>(&id_fn.body->value);
  REQUIRE(id_body != nullptr);
  REQUIRE(id_body->statements.size() == 1);
  auto const id_ret =
    std::get_if<benson::ast::Return_statement>(&id_body->statements[0].value);
  REQUIRE(id_ret != nullptr);
  auto const id_ret_val =
    std::get_if<benson::ast::Identifier_expression>(&id_ret->value.value);
  REQUIRE(id_ret_val != nullptr);
  CHECK(spellings.lookup(id_ret_val->identifier.spelling) == "x");
  // first: fn(x: Int32, y: Int32): Int32 => { return x; }
  REQUIRE(first_fn.parameters.size() == 2);
  CHECK(spellings.lookup(first_fn.parameters[0].name.spelling) == "x");
  auto const first_param0_type =
    std::get_if<benson::ast::Identifier_expression>(
      &first_fn.parameters[0].type->value
    );
  REQUIRE(first_param0_type != nullptr);
  CHECK(spellings.lookup(first_param0_type->identifier.spelling) == "Int32");
  CHECK(spellings.lookup(first_fn.parameters[1].name.spelling) == "y");
  auto const first_param1_type =
    std::get_if<benson::ast::Identifier_expression>(
      &first_fn.parameters[1].type->value
    );
  REQUIRE(first_param1_type != nullptr);
  CHECK(spellings.lookup(first_param1_type->identifier.spelling) == "Int32");
  auto const first_ret_type = std::get_if<benson::ast::Identifier_expression>(
    &first_fn.return_type_specifier.type->value
  );
  REQUIRE(first_ret_type != nullptr);
  CHECK(spellings.lookup(first_ret_type->identifier.spelling) == "Int32");
  auto const first_body =
    std::get_if<benson::ast::Block_expression>(&first_fn.body->value);
  REQUIRE(first_body != nullptr);
  REQUIRE(first_body->statements.size() == 1);
  auto const first_ret = std::get_if<benson::ast::Return_statement>(
    &first_body->statements[0].value
  );
  REQUIRE(first_ret != nullptr);
  auto const first_ret_val =
    std::get_if<benson::ast::Identifier_expression>(&first_ret->value.value);
  REQUIRE(first_ret_val != nullptr);
  CHECK(spellings.lookup(first_ret_val->identifier.spelling) == "x");
  // main: fn(): Int32 => { return first(id(42), 0); }
  auto const main_ret_type = std::get_if<benson::ast::Identifier_expression>(
    &main_fn.return_type_specifier.type->value
  );
  REQUIRE(main_ret_type != nullptr);
  CHECK(spellings.lookup(main_ret_type->identifier.spelling) == "Int32");
  auto const main_body =
    std::get_if<benson::ast::Block_expression>(&main_fn.body->value);
  REQUIRE(main_body != nullptr);
  REQUIRE(main_body->statements.size() == 1);
  auto const main_ret =
    std::get_if<benson::ast::Return_statement>(&main_body->statements[0].value);
  REQUIRE(main_ret != nullptr);
  // first(id(42), 0) — outer call
  auto const outer_call =
    std::get_if<benson::ast::Call_expression>(&main_ret->value.value);
  REQUIRE(outer_call != nullptr);
  auto const outer_callee =
    std::get_if<benson::ast::Identifier_expression>(&outer_call->callee->value);
  REQUIRE(outer_callee != nullptr);
  CHECK(spellings.lookup(outer_callee->identifier.spelling) == "first");
  REQUIRE(outer_call->arguments.size() == 2);
  auto const inner_call =
    std::get_if<benson::ast::Call_expression>(&outer_call->arguments[0].value);
  REQUIRE(inner_call != nullptr);
  auto const inner_callee =
    std::get_if<benson::ast::Identifier_expression>(&inner_call->callee->value);
  REQUIRE(inner_callee != nullptr);
  CHECK(spellings.lookup(inner_callee->identifier.spelling) == "id");
  REQUIRE(inner_call->arguments.size() == 1);
  auto const inner_arg = std::get_if<benson::ast::Int_literal_expression>(
    &inner_call->arguments[0].value
  );
  REQUIRE(inner_arg != nullptr);
  CHECK(spellings.lookup(inner_arg->literal.spelling) == "42");
  auto const outer_arg1 = std::get_if<benson::ast::Int_literal_expression>(
    &outer_call->arguments[1].value
  );
  REQUIRE(outer_arg1 != nullptr);
  CHECK(spellings.lookup(outer_arg1->literal.spelling) == "0");
}

TEST_CASE("Parser - call_expression.benson parses successfully")
{
  auto file =
    std::ifstream{std::string{EXAMPLES_PATH} + "/call_expression.benson"};
  auto binary_stream = benson::Istream_binary_stream{&file};
  auto char_stream = benson::Utf8_char_stream{&binary_stream};
  auto spellings = benson::Spelling_table{};
  auto lexeme_stream = benson::Lexeme_stream{&char_stream, &spellings};
  auto reader = benson::Lexeme_stream_reader{&lexeme_stream};
  auto parser = benson::Parser{&reader, &spellings};
  auto const unit = parser.parse_translation_unit();
  REQUIRE(unit.statements.size() == 2);
  auto const &foo_fn = top_level_fn(unit.statements[0]);
  auto const &main_fn = top_level_fn(unit.statements[1]);
  REQUIRE(foo_fn.name.has_value());
  REQUIRE(main_fn.name.has_value());
  CHECK(spellings.lookup(foo_fn.name->spelling) == "foo");
  CHECK(spellings.lookup(main_fn.name->spelling) == "main");
  // foo returns (fn(): Int32 => { return 0; })() — a call with a paren-wrapped
  // fn expression callee
  auto const foo_body =
    std::get_if<benson::ast::Block_expression>(&foo_fn.body->value);
  REQUIRE(foo_body != nullptr);
  REQUIRE(foo_body->statements.size() == 1);
  auto const foo_ret =
    std::get_if<benson::ast::Return_statement>(&foo_body->statements[0].value);
  REQUIRE(foo_ret != nullptr);
  auto const foo_call =
    std::get_if<benson::ast::Call_expression>(&foo_ret->value.value);
  REQUIRE(foo_call != nullptr);
  CHECK(spellings.lookup(foo_call->lparen.spelling) == "(");
  CHECK(spellings.lookup(foo_call->rparen.spelling) == ")");
  auto const foo_paren =
    std::get_if<benson::ast::Paren_expression>(&foo_call->callee->value);
  REQUIRE(foo_paren != nullptr);
  CHECK(
    std::get_if<benson::ast::Fn_expression>(&foo_paren->inner->value) != nullptr
  );
  // main returns foo() — a call with an identifier callee
  auto const main_body =
    std::get_if<benson::ast::Block_expression>(&main_fn.body->value);
  REQUIRE(main_body != nullptr);
  REQUIRE(main_body->statements.size() == 1);
  auto const main_ret =
    std::get_if<benson::ast::Return_statement>(&main_body->statements[0].value);
  REQUIRE(main_ret != nullptr);
  auto const main_call =
    std::get_if<benson::ast::Call_expression>(&main_ret->value.value);
  REQUIRE(main_call != nullptr);
  CHECK(spellings.lookup(main_call->lparen.spelling) == "(");
  CHECK(spellings.lookup(main_call->rparen.spelling) == ")");
  auto const callee =
    std::get_if<benson::ast::Identifier_expression>(&main_call->callee->value);
  REQUIRE(callee != nullptr);
  CHECK(spellings.lookup(callee->identifier.spelling) == "foo");
}

TEST_CASE("Parser - accepts valid code")
{
  CHECK(parses(""));
  CHECK(parses("let main = fn(): Int32 => { return 0; };"));
  CHECK(parses("let main = fn(): Int32 => { };"));
  CHECK(parses(
    "let main = fn(): Int32 => { return 0; };\n"
    "let other = fn(): Int32 => { return 1; };"
  ));
  CHECK(parses("let main = fn(): Void => { x; };"));
  CHECK(parses("let main = fn(): Void => { let x = 42; };"));
  CHECK(parses("let main = fn(): Void => { (x); };"));
  CHECK(parses("let main = fn(): Void => { ((42)); };"));
  CHECK(parses("let f = fn(x: Int32): Int32 => { return x; };"));
  CHECK(parses("let f = fn(x: Int32, y: Int32): Int32 => { return x; };"));
  CHECK(parses("let f = fn(x: Int32,): Int32 => { return x; };"));
  CHECK(parses("let f = fn(mut x: Int32): Void => { };"));
  CHECK(parses("let f = fn(x: Int32, mut y: Int32): Void => { };"));
  CHECK(parses("let main = fn(): Void => { f(1); };"));
  CHECK(parses("let main = fn(): Void => { f(1, 2); };"));
  CHECK(parses("let main = fn(): Void => { f(1,); };"));
  CHECK(parses("let x = 42;"));
  // named function expression syntax
  CHECK(parses("fn main(): Int32 => { return 0; };"));
  CHECK(parses("fn add(x: Int32, y: Int32): Int32 => x + y;"));
  CHECK(parses("fn f(n: Int32): Int32 => f(n);"));
}

TEST_CASE("Parser - rejects invalid code")
{
  CHECK_FALSE(parses("let"));
  CHECK_FALSE(parses("let x"));
  CHECK_FALSE(parses("let x ="));
  CHECK_FALSE(parses("let x = fn()"));
  CHECK_FALSE(parses("let x = fn() =>"));
  CHECK_FALSE(parses("let x = fn() => Int32"));
  CHECK_FALSE(parses("let x = fn(): Int32 => {"));
  CHECK_FALSE(parses("let x = fn(): Int32 => { }"));
  CHECK_FALSE(parses("{"));
  CHECK_FALSE(parses("42"));
  CHECK_FALSE(parses("let = fn(): Int32 => { };"));
  CHECK_FALSE(parses("let x = fn(): Int32 => { return; };"));
  CHECK_FALSE(parses("let x = fn(): Int32 => { return (; };"));
  CHECK_FALSE(parses("let x = fn(): Int32 => { return (); };"));
  // malformed parameter declarations
  CHECK_FALSE(parses("let f = fn(x): Int32 => { };"));
  CHECK_FALSE(parses("let f = fn(x:): Int32 => { };"));
  CHECK_FALSE(parses("let f = fn(x: Int32 y: Int32): Int32 => { };"));
  // malformed argument lists
  CHECK_FALSE(parses("let main = fn(): Void => { f(,); };"));
  CHECK_FALSE(parses("let main = fn(): Void => { f(1 2); };"));
  // malformed expressions: missing operands
  CHECK_FALSE(parses("let x = fn(): Int32 => { return 1 +; };"));
  CHECK_FALSE(parses("let x = fn(): Int32 => { return 1 *; };"));
  CHECK_FALSE(parses("let x = fn(): Int32 => { return * 1; };"));
  CHECK_FALSE(parses("let x = fn(): Int32 => { return 1 + * 2; };"));
  CHECK_FALSE(parses("let x = fn(): Int32 => { return +; };"));
  CHECK_FALSE(parses("let x = fn(): Int32 => { return -; };"));
  // malformed array types
  CHECK_FALSE(parses("let f = fn(x: [4]): Void => { };"));
  CHECK_FALSE(parses("let f = fn(x: Int32[4): Void => { };"));
  CHECK_FALSE(parses("let f = fn(x: [4]): Void => { };"));
}

TEST_CASE("parse_translation_unit - empty")
{
  auto fixture = Parse_fixture{""};
  auto const unit = fixture.parser.parse_translation_unit();
  CHECK(unit.statements.empty());
}

TEST_CASE("parse_translation_unit - multiple let statements")
{
  auto fixture = Parse_fixture{"let a = fn(): Int32 => { return 1; };\n"
                               "let b = fn(): Int32 => { return 2; };"};
  auto const &spellings = fixture.spellings;
  auto const unit = fixture.parser.parse_translation_unit();
  REQUIRE(unit.statements.size() == 2);
  auto const &stmt_a =
    std::get<benson::ast::Let_statement>(unit.statements[0].value);
  auto const &stmt_b =
    std::get<benson::ast::Let_statement>(unit.statements[1].value);
  CHECK(spellings.lookup(stmt_a.name.spelling) == "a");
  CHECK(spellings.lookup(stmt_b.name.spelling) == "b");
}

TEST_CASE("parse_let_statement - function declaration")
{
  auto fixture = Parse_fixture{"let main = fn(): Int32 => { return 0; };"};
  auto const &spellings = fixture.spellings;
  auto const stmt = fixture.parser.parse_let_statement();
  CHECK(spellings.lookup(stmt.kw_let.spelling) == "let");
  CHECK(spellings.lookup(stmt.name.spelling) == "main");
  CHECK(spellings.lookup(stmt.eq.spelling) == "=");
  auto const fn =
    std::get_if<benson::ast::Fn_expression>(&stmt.initializer.value);
  REQUIRE(fn != nullptr);
  CHECK(spellings.lookup(fn->kw_fn.spelling) == "fn");
  REQUIRE(fn->body != nullptr);
  auto const body =
    std::get_if<benson::ast::Block_expression>(&fn->body->value);
  REQUIRE(body != nullptr);
  CHECK(body->statements.size() == 1);
}

TEST_CASE("parse_let_statement")
{
  auto fixture = Parse_fixture{"let x = 42;"};
  auto const &spellings = fixture.spellings;
  auto const stmt = fixture.parser.parse_let_statement();
  CHECK(spellings.lookup(stmt.kw_let.spelling) == "let");
  CHECK(spellings.lookup(stmt.name.spelling) == "x");
  CHECK(spellings.lookup(stmt.eq.spelling) == "=");
  auto const lit =
    std::get_if<benson::ast::Int_literal_expression>(&stmt.initializer.value);
  REQUIRE(lit != nullptr);
  CHECK(spellings.lookup(lit->literal.spelling) == "42");
  CHECK(spellings.lookup(stmt.semicolon.spelling) == ";");
}

TEST_CASE("parse_return_statement")
{
  auto fixture = Parse_fixture{"return 99;"};
  auto const &spellings = fixture.spellings;
  auto const stmt = fixture.parser.parse_return_statement();
  CHECK(spellings.lookup(stmt.kw_return.spelling) == "return");
  auto const lit =
    std::get_if<benson::ast::Int_literal_expression>(&stmt.value.value);
  REQUIRE(lit != nullptr);
  CHECK(spellings.lookup(lit->literal.spelling) == "99");
  CHECK(spellings.lookup(stmt.semicolon.spelling) == ";");
}

TEST_CASE("parse_expression_statement")
{
  auto fixture = Parse_fixture{"foo;"};
  auto const &spellings = fixture.spellings;
  auto const stmt = fixture.parser.parse_expression_statement();
  auto const id =
    std::get_if<benson::ast::Identifier_expression>(&stmt.expression.value);
  REQUIRE(id != nullptr);
  CHECK(spellings.lookup(id->identifier.spelling) == "foo");
  CHECK(spellings.lookup(stmt.semicolon.spelling) == ";");
}

TEST_CASE("parse_block_expression")
{
  auto fixture = Parse_fixture{"{ return 1; let x = 2; }"};
  auto const &spellings = fixture.spellings;
  auto const block = fixture.parser.parse_block_expression();
  CHECK(spellings.lookup(block.lbrace.spelling) == "{");
  REQUIRE(block.statements.size() == 2);
  CHECK(
    std::get_if<benson::ast::Return_statement>(&block.statements[0].value) !=
    nullptr
  );
  CHECK(
    std::get_if<benson::ast::Let_statement>(&block.statements[1].value) !=
    nullptr
  );
  CHECK(spellings.lookup(block.rbrace.spelling) == "}");
}

TEST_CASE("parse_block_expression - empty")
{
  auto fixture = Parse_fixture{"{ }"};
  auto const &spellings = fixture.spellings;
  auto const block = fixture.parser.parse_block_expression();
  CHECK(spellings.lookup(block.lbrace.spelling) == "{");
  CHECK(block.statements.empty());
  CHECK(block.tail == nullptr);
  CHECK(spellings.lookup(block.rbrace.spelling) == "}");
}

TEST_CASE("parse_block_expression - tail expression")
{
  auto fixture = Parse_fixture{"{ 42 }"};
  auto const &spellings = fixture.spellings;
  auto const block = fixture.parser.parse_block_expression();
  CHECK(block.statements.empty());
  REQUIRE(block.tail != nullptr);
  auto const lit =
    std::get_if<benson::ast::Int_literal_expression>(&block.tail->value);
  REQUIRE(lit != nullptr);
  CHECK(spellings.lookup(lit->literal.spelling) == "42");
}

TEST_CASE("parse_block_expression - statements then tail")
{
  auto fixture = Parse_fixture{"{ let x = 1; x + 1 }"};
  auto const &spellings = fixture.spellings;
  auto const block = fixture.parser.parse_block_expression();
  REQUIRE(block.statements.size() == 1);
  CHECK(
    std::get_if<benson::ast::Let_statement>(&block.statements[0].value) !=
    nullptr
  );
  REQUIRE(block.tail != nullptr);
  auto const bin =
    std::get_if<benson::ast::Binary_expression>(&block.tail->value);
  REQUIRE(bin != nullptr);
  CHECK(spellings.lookup(bin->op.spelling) == "+");
}

TEST_CASE("parse_block_expression - no tail with semicolon")
{
  auto fixture = Parse_fixture{"{ x; }"};
  auto const block = fixture.parser.parse_block_expression();
  REQUIRE(block.statements.size() == 1);
  CHECK(block.tail == nullptr);
}

TEST_CASE("parse_expression - block as primary expression")
{
  auto fixture = Parse_fixture{"{ 1 + 2 }"};
  auto const expr = fixture.parser.parse_expression();
  auto const block = std::get_if<benson::ast::Block_expression>(&expr->value);
  REQUIRE(block != nullptr);
  REQUIRE(block->tail != nullptr);
  CHECK(
    std::get_if<benson::ast::Binary_expression>(&block->tail->value) != nullptr
  );
}

TEST_CASE("parse_expression - nested blocks")
{
  auto fixture = Parse_fixture{"{{{{}}}}"};
  auto const expr = fixture.parser.parse_expression();
  // outer block: no statements, tail is a block
  auto const b0 = std::get_if<benson::ast::Block_expression>(&expr->value);
  REQUIRE(b0 != nullptr);
  CHECK(b0->statements.empty());
  REQUIRE(b0->tail != nullptr);
  auto const b1 = std::get_if<benson::ast::Block_expression>(&b0->tail->value);
  REQUIRE(b1 != nullptr);
  CHECK(b1->statements.empty());
  REQUIRE(b1->tail != nullptr);
  auto const b2 = std::get_if<benson::ast::Block_expression>(&b1->tail->value);
  REQUIRE(b2 != nullptr);
  CHECK(b2->statements.empty());
  REQUIRE(b2->tail != nullptr);
  auto const b3 = std::get_if<benson::ast::Block_expression>(&b2->tail->value);
  REQUIRE(b3 != nullptr);
  CHECK(b3->statements.empty());
  CHECK(b3->tail == nullptr);
}

TEST_CASE("parse_expression - block as expression statement")
{
  CHECK(parses("let f = fn(): Void => { { 42 }; };"));
}

TEST_CASE("parse_expression - block as initializer")
{
  CHECK(parses("let f = fn(): Void => { let x = { let a = 1; a + 1 }; };"));
}

TEST_CASE("parse_block_expression - fn body parses tail syntactically")
{
  auto fixture = Parse_fixture{"fn(): Int32 => { 42 }"};
  auto const &spellings = fixture.spellings;
  auto const fn = fixture.parser.parse_fn_expression();
  REQUIRE(fn.body != nullptr);
  auto const body = std::get_if<benson::ast::Block_expression>(&fn.body->value);
  REQUIRE(body != nullptr);
  CHECK(body->statements.empty());
  REQUIRE(body->tail != nullptr);
  auto const lit =
    std::get_if<benson::ast::Int_literal_expression>(&body->tail->value);
  REQUIRE(lit != nullptr);
  CHECK(spellings.lookup(lit->literal.spelling) == "42");
}

TEST_CASE("parse_fn_expression")
{
  auto fixture = Parse_fixture{"fn(): Int32 => { return 0; }"};
  auto const &spellings = fixture.spellings;
  auto const fn = fixture.parser.parse_fn_expression();
  CHECK(spellings.lookup(fn.kw_fn.spelling) == "fn");
  CHECK(spellings.lookup(fn.lparen.spelling) == "(");
  CHECK(spellings.lookup(fn.rparen.spelling) == ")");
  CHECK(spellings.lookup(fn.return_type_specifier.colon.spelling) == ":");
  auto const ret_type = std::get_if<benson::ast::Identifier_expression>(
    &fn.return_type_specifier.type->value
  );
  REQUIRE(ret_type != nullptr);
  CHECK(spellings.lookup(ret_type->identifier.spelling) == "Int32");
  CHECK(spellings.lookup(fn.arrow.spelling) == "=>");
  REQUIRE(fn.body != nullptr);
  auto const body = std::get_if<benson::ast::Block_expression>(&fn.body->value);
  REQUIRE(body != nullptr);
  CHECK(body->statements.size() == 1);
}

TEST_CASE("parse_fn_expression - mut parameter")
{
  auto fixture = Parse_fixture{"fn(mut x: Int32): Void => { }"};
  auto const &spellings = fixture.spellings;
  auto const fn = fixture.parser.parse_fn_expression();
  REQUIRE(fn.parameters.size() == 1);
  REQUIRE(fn.parameters[0].kw_mut.has_value());
  CHECK(spellings.lookup(fn.parameters[0].kw_mut->spelling) == "mut");
  CHECK(spellings.lookup(fn.parameters[0].name.spelling) == "x");
}

TEST_CASE("parse_fn_expression - mixed mut and non-mut parameters")
{
  auto fixture = Parse_fixture{"fn(x: Int32, mut y: Int32): Void => { }"};
  auto const &spellings = fixture.spellings;
  auto const fn = fixture.parser.parse_fn_expression();
  REQUIRE(fn.parameters.size() == 2);
  CHECK_FALSE(fn.parameters[0].kw_mut.has_value());
  CHECK(spellings.lookup(fn.parameters[0].name.spelling) == "x");
  REQUIRE(fn.parameters[1].kw_mut.has_value());
  CHECK(spellings.lookup(fn.parameters[1].name.spelling) == "y");
}

TEST_CASE("parse_fn_expression - requires return type")
{
  auto fixture = Parse_fixture{"fn() => { }"};
  CHECK_THROWS(fixture.parser.parse_fn_expression());
}

TEST_CASE("parse_int_literal_expression")
{
  auto fixture = Parse_fixture{"42"};
  auto const &spellings = fixture.spellings;
  auto const expr = fixture.parser.parse_int_literal_expression();
  CHECK(spellings.lookup(expr.literal.spelling) == "42");
}

TEST_CASE("parse_identifier_expression")
{
  auto fixture = Parse_fixture{"foo"};
  auto const &spellings = fixture.spellings;
  auto const expr = fixture.parser.parse_identifier_expression();
  CHECK(spellings.lookup(expr.identifier.spelling) == "foo");
}

TEST_CASE("parse_paren_expression")
{
  auto fixture = Parse_fixture{"(42)"};
  auto const &spellings = fixture.spellings;
  auto const expr = fixture.parser.parse_paren_expression();
  CHECK(spellings.lookup(expr.lparen.spelling) == "(");
  auto const inner =
    std::get_if<benson::ast::Int_literal_expression>(&expr.inner->value);
  REQUIRE(inner != nullptr);
  CHECK(spellings.lookup(inner->literal.spelling) == "42");
  CHECK(spellings.lookup(expr.rparen.spelling) == ")");
}

TEST_CASE("parse_paren_expression - nested")
{
  auto fixture = Parse_fixture{"((x))"};
  auto const &spellings = fixture.spellings;
  auto const expr = fixture.parser.parse_paren_expression();
  CHECK(spellings.lookup(expr.lparen.spelling) == "(");
  auto const inner =
    std::get_if<benson::ast::Paren_expression>(&expr.inner->value);
  REQUIRE(inner != nullptr);
  auto const id =
    std::get_if<benson::ast::Identifier_expression>(&inner->inner->value);
  REQUIRE(id != nullptr);
  CHECK(spellings.lookup(id->identifier.spelling) == "x");
  CHECK(spellings.lookup(inner->rparen.spelling) == ")");
  CHECK(spellings.lookup(expr.rparen.spelling) == ")");
}

TEST_CASE("parse_primary_expression - dispatches to int literal")
{
  auto fixture = Parse_fixture{"123"};
  auto const expr = fixture.parser.parse_primary_expression();
  CHECK(
    std::get_if<benson::ast::Int_literal_expression>(&expr->value) != nullptr
  );
}

TEST_CASE("parse_primary_expression - dispatches to identifier")
{
  auto fixture = Parse_fixture{"x"};
  auto const expr = fixture.parser.parse_primary_expression();
  CHECK(
    std::get_if<benson::ast::Identifier_expression>(&expr->value) != nullptr
  );
}

TEST_CASE("parse_primary_expression - dispatches to fn")
{
  auto fixture = Parse_fixture{"fn(): Int32 => { }"};
  auto const expr = fixture.parser.parse_primary_expression();
  CHECK(std::get_if<benson::ast::Fn_expression>(&expr->value) != nullptr);
}

TEST_CASE("parse_primary_expression - dispatches to paren")
{
  auto fixture = Parse_fixture{"(42)"};
  auto const expr = fixture.parser.parse_primary_expression();
  CHECK(std::get_if<benson::ast::Paren_expression>(&expr->value) != nullptr);
}

TEST_CASE("parse_expression - simple binary expression")
{
  auto fixture = Parse_fixture{"1 + 2"};
  auto const &spellings = fixture.spellings;
  auto const expr = fixture.parser.parse_expression();
  auto const bin = std::get_if<benson::ast::Binary_expression>(&expr->value);
  REQUIRE(bin != nullptr);
  auto const left =
    std::get_if<benson::ast::Int_literal_expression>(&bin->left->value);
  REQUIRE(left != nullptr);
  CHECK(spellings.lookup(left->literal.spelling) == "1");
  CHECK(spellings.lookup(bin->op.spelling) == "+");
  auto const right =
    std::get_if<benson::ast::Int_literal_expression>(&bin->right->value);
  REQUIRE(right != nullptr);
  CHECK(spellings.lookup(right->literal.spelling) == "2");
}

TEST_CASE("parse_expression - multiplicative before additive")
{
  // 1 + 2 * 3 should parse as 1 + (2 * 3)
  auto fixture = Parse_fixture{"1 + 2 * 3"};
  auto const &spellings = fixture.spellings;
  auto const expr = fixture.parser.parse_expression();
  auto const add = std::get_if<benson::ast::Binary_expression>(&expr->value);
  REQUIRE(add != nullptr);
  CHECK(spellings.lookup(add->op.spelling) == "+");
  auto const mul =
    std::get_if<benson::ast::Binary_expression>(&add->right->value);
  REQUIRE(mul != nullptr);
  CHECK(spellings.lookup(mul->op.spelling) == "*");
  auto const two =
    std::get_if<benson::ast::Int_literal_expression>(&mul->left->value);
  REQUIRE(two != nullptr);
  CHECK(spellings.lookup(two->literal.spelling) == "2");
  auto const three =
    std::get_if<benson::ast::Int_literal_expression>(&mul->right->value);
  REQUIRE(three != nullptr);
  CHECK(spellings.lookup(three->literal.spelling) == "3");
}

TEST_CASE("parse_expression - left associativity")
{
  // 1 - 2 - 3 should parse as (1 - 2) - 3
  auto fixture = Parse_fixture{"1 - 2 - 3"};
  auto const &spellings = fixture.spellings;
  auto const expr = fixture.parser.parse_expression();
  auto const outer = std::get_if<benson::ast::Binary_expression>(&expr->value);
  REQUIRE(outer != nullptr);
  CHECK(spellings.lookup(outer->op.spelling) == "-");
  auto const inner =
    std::get_if<benson::ast::Binary_expression>(&outer->left->value);
  REQUIRE(inner != nullptr);
  CHECK(spellings.lookup(inner->op.spelling) == "-");
  auto const three =
    std::get_if<benson::ast::Int_literal_expression>(&outer->right->value);
  REQUIRE(three != nullptr);
  CHECK(spellings.lookup(three->literal.spelling) == "3");
}

TEST_CASE("parse_expression - all operators")
{
  // -f() + +2 - 3 * 4 / 5 % 6
  // parses as (-(f()) + (+2)) - (((3 * 4) / 5) % 6)
  auto fixture = Parse_fixture{"-f() + +2 - 3 * 4 / 5 % 6"};
  auto const &spellings = fixture.spellings;
  auto const expr = fixture.parser.parse_expression();
  // outer: (-f() + +2) - (...)
  auto const sub = std::get_if<benson::ast::Binary_expression>(&expr->value);
  REQUIRE(sub != nullptr);
  CHECK(spellings.lookup(sub->op.spelling) == "-");
  // left of -: -f() + +2
  auto const add =
    std::get_if<benson::ast::Binary_expression>(&sub->left->value);
  REQUIRE(add != nullptr);
  CHECK(spellings.lookup(add->op.spelling) == "+");
  // left of +: -f()
  auto const unary_minus =
    std::get_if<benson::ast::Prefix_expression>(&add->left->value);
  REQUIRE(unary_minus != nullptr);
  CHECK(spellings.lookup(unary_minus->op.spelling) == "-");
  auto const call =
    std::get_if<benson::ast::Call_expression>(&unary_minus->operand->value);
  REQUIRE(call != nullptr);
  auto const callee =
    std::get_if<benson::ast::Identifier_expression>(&call->callee->value);
  REQUIRE(callee != nullptr);
  CHECK(spellings.lookup(callee->identifier.spelling) == "f");
  // right of +: +2
  auto const unary_plus =
    std::get_if<benson::ast::Prefix_expression>(&add->right->value);
  REQUIRE(unary_plus != nullptr);
  CHECK(spellings.lookup(unary_plus->op.spelling) == "+");
  // right of -: ((3 * 4) / 5) % 6
  auto const mod =
    std::get_if<benson::ast::Binary_expression>(&sub->right->value);
  REQUIRE(mod != nullptr);
  CHECK(spellings.lookup(mod->op.spelling) == "%");
  // left of %: (3 * 4) / 5
  auto const div =
    std::get_if<benson::ast::Binary_expression>(&mod->left->value);
  REQUIRE(div != nullptr);
  CHECK(spellings.lookup(div->op.spelling) == "/");
  // left of /: 3 * 4
  auto const mul =
    std::get_if<benson::ast::Binary_expression>(&div->left->value);
  REQUIRE(mul != nullptr);
  CHECK(spellings.lookup(mul->op.spelling) == "*");
}

TEST_CASE("parse_expression - call binds tighter than binary op")
{
  // f() + 1 should parse as (f()) + 1
  auto fixture = Parse_fixture{"f() + 1"};
  auto const &spellings = fixture.spellings;
  auto const expr = fixture.parser.parse_expression();
  auto const add = std::get_if<benson::ast::Binary_expression>(&expr->value);
  REQUIRE(add != nullptr);
  CHECK(spellings.lookup(add->op.spelling) == "+");
  CHECK(
    std::get_if<benson::ast::Call_expression>(&add->left->value) != nullptr
  );
}

TEST_CASE("parse_statement - dispatches to let")
{
  auto fixture = Parse_fixture{"let x = 1;"};
  auto const stmt = fixture.parser.parse_statement();
  CHECK(std::get_if<benson::ast::Let_statement>(&stmt.value) != nullptr);
}

TEST_CASE("parse_statement - dispatches to return")
{
  auto fixture = Parse_fixture{"return 1;"};
  auto const stmt = fixture.parser.parse_statement();
  CHECK(std::get_if<benson::ast::Return_statement>(&stmt.value) != nullptr);
}

TEST_CASE("parse_statement - dispatches to expression statement")
{
  auto fixture = Parse_fixture{"x;"};
  auto const stmt = fixture.parser.parse_statement();
  CHECK(std::get_if<benson::ast::Expression_statement>(&stmt.value) != nullptr);
}

TEST_CASE("parse_expression - unary minus: call binds tighter")
{
  // -f() should parse as -(f()), not (-f)()
  auto fixture = Parse_fixture{"-f()"};
  auto const &spellings = fixture.spellings;
  auto const expr = fixture.parser.parse_expression();
  auto const unary = std::get_if<benson::ast::Prefix_expression>(&expr->value);
  REQUIRE(unary != nullptr);
  CHECK(spellings.lookup(unary->op.spelling) == "-");
  CHECK(
    std::get_if<benson::ast::Call_expression>(&unary->operand->value) != nullptr
  );
}

TEST_CASE("parse_fn_expression - array parameter")
{
  auto fixture = Parse_fixture{"fn(buf: Int32[4]): Void => { }"};
  auto const &spellings = fixture.spellings;
  auto const fn = fixture.parser.parse_fn_expression();
  REQUIRE(fn.parameters.size() == 1);
  CHECK(spellings.lookup(fn.parameters[0].name.spelling) == "buf");
  auto const param_type =
    std::get_if<benson::ast::Index_expression>(&fn.parameters[0].type->value);
  REQUIRE(param_type != nullptr);
  auto const elem = std::get_if<benson::ast::Identifier_expression>(
    &param_type->operand->value
  );
  REQUIRE(elem != nullptr);
  CHECK(spellings.lookup(elem->identifier.spelling) == "Int32");
  auto const size =
    std::get_if<benson::ast::Int_literal_expression>(&param_type->index->value);
  REQUIRE(size != nullptr);
  CHECK(spellings.lookup(size->literal.spelling) == "4");
}

TEST_CASE("parse_expression - index: simple")
{
  auto fixture = Parse_fixture{"arr[0]"};
  auto const &spellings = fixture.spellings;
  auto const expr = fixture.parser.parse_expression();
  auto const idx = std::get_if<benson::ast::Index_expression>(&expr->value);
  REQUIRE(idx != nullptr);
  auto const operand =
    std::get_if<benson::ast::Identifier_expression>(&idx->operand->value);
  REQUIRE(operand != nullptr);
  CHECK(spellings.lookup(operand->identifier.spelling) == "arr");
  CHECK(spellings.lookup(idx->lbracket.spelling) == "[");
  auto const index =
    std::get_if<benson::ast::Int_literal_expression>(&idx->index->value);
  REQUIRE(index != nullptr);
  CHECK(spellings.lookup(index->literal.spelling) == "0");
  CHECK(spellings.lookup(idx->rbracket.spelling) == "]");
}

TEST_CASE("parse_expression - index: chained")
{
  // arr[0][1] should parse as (arr[0])[1]
  auto fixture = Parse_fixture{"arr[0][1]"};
  auto const &spellings = fixture.spellings;
  auto const expr = fixture.parser.parse_expression();
  auto const outer = std::get_if<benson::ast::Index_expression>(&expr->value);
  REQUIRE(outer != nullptr);
  auto const outer_index =
    std::get_if<benson::ast::Int_literal_expression>(&outer->index->value);
  REQUIRE(outer_index != nullptr);
  CHECK(spellings.lookup(outer_index->literal.spelling) == "1");
  auto const inner =
    std::get_if<benson::ast::Index_expression>(&outer->operand->value);
  REQUIRE(inner != nullptr);
  auto const inner_operand =
    std::get_if<benson::ast::Identifier_expression>(&inner->operand->value);
  REQUIRE(inner_operand != nullptr);
  CHECK(spellings.lookup(inner_operand->identifier.spelling) == "arr");
  auto const inner_index =
    std::get_if<benson::ast::Int_literal_expression>(&inner->index->value);
  REQUIRE(inner_index != nullptr);
  CHECK(spellings.lookup(inner_index->literal.spelling) == "0");
}

TEST_CASE("parse_expression - index: expression index")
{
  auto fixture = Parse_fixture{"arr[i + 1]"};
  auto const &spellings = fixture.spellings;
  auto const expr = fixture.parser.parse_expression();
  auto const idx = std::get_if<benson::ast::Index_expression>(&expr->value);
  REQUIRE(idx != nullptr);
  auto const index =
    std::get_if<benson::ast::Binary_expression>(&idx->index->value);
  REQUIRE(index != nullptr);
  CHECK(spellings.lookup(index->op.spelling) == "+");
}

TEST_CASE("parse_expression - index: binds tighter than binary op")
{
  // arr[0] + 1 should parse as (arr[0]) + 1
  auto fixture = Parse_fixture{"arr[0] + 1"};
  auto const &spellings = fixture.spellings;
  auto const expr = fixture.parser.parse_expression();
  auto const add = std::get_if<benson::ast::Binary_expression>(&expr->value);
  REQUIRE(add != nullptr);
  CHECK(spellings.lookup(add->op.spelling) == "+");
  CHECK(
    std::get_if<benson::ast::Index_expression>(&add->left->value) != nullptr
  );
}

TEST_CASE("parse_expression - index: call then index")
{
  // f()[0] should parse as (f())[0]
  auto fixture = Parse_fixture{"f()[0]"};
  auto const expr = fixture.parser.parse_expression();
  auto const idx = std::get_if<benson::ast::Index_expression>(&expr->value);
  REQUIRE(idx != nullptr);
  CHECK(
    std::get_if<benson::ast::Call_expression>(&idx->operand->value) != nullptr
  );
}

TEST_CASE("parse_expression - index: in full program")
{
  CHECK(parses("let f = fn(buf: Int32[4]): Int32 => { return buf[0]; };"));
  CHECK(parses("let f = fn(): Void => { arr[0]; };"));
  CHECK(parses("let f = fn(): Void => { arr[i + 1]; };"));
  CHECK(parses("let f = fn(): Void => { f()[0]; };"));
  CHECK(parses("let f = fn(): Void => { arr[0][1]; };"));
}

TEST_CASE("parse_expression - index: rejects")
{
  CHECK_FALSE(parses("let f = fn(): Void => { arr[0; };"));
}

TEST_CASE("parse_expression - dereference")
{
  auto fixture = Parse_fixture{"p^"};
  auto const &spellings = fixture.spellings;
  auto const expr = fixture.parser.parse_expression();
  auto const postfix =
    std::get_if<benson::ast::Postfix_expression>(&expr->value);
  REQUIRE(postfix != nullptr);
  CHECK(spellings.lookup(postfix->op.spelling) == "^");
  auto const operand =
    std::get_if<benson::ast::Identifier_expression>(&postfix->operand->value);
  REQUIRE(operand != nullptr);
  CHECK(spellings.lookup(operand->identifier.spelling) == "p");
}

TEST_CASE("parse_expression - dereference: index then deref")
{
  // p[0]^ should parse as (p[0])^
  auto fixture = Parse_fixture{"p[0]^"};
  auto const &spellings = fixture.spellings;
  auto const expr = fixture.parser.parse_expression();
  auto const postfix =
    std::get_if<benson::ast::Postfix_expression>(&expr->value);
  REQUIRE(postfix != nullptr);
  CHECK(spellings.lookup(postfix->op.spelling) == "^");
  CHECK(
    std::get_if<benson::ast::Index_expression>(&postfix->operand->value) !=
    nullptr
  );
}

TEST_CASE("parse_expression - dereference: in binary expression")
{
  // a^ + b^ should parse as (a^) + (b^)
  auto fixture = Parse_fixture{"a^ + b^"};
  auto const &spellings = fixture.spellings;
  auto const expr = fixture.parser.parse_expression();
  auto const add = std::get_if<benson::ast::Binary_expression>(&expr->value);
  REQUIRE(add != nullptr);
  CHECK(spellings.lookup(add->op.spelling) == "+");
  auto const left =
    std::get_if<benson::ast::Postfix_expression>(&add->left->value);
  REQUIRE(left != nullptr);
  CHECK(spellings.lookup(left->op.spelling) == "^");
  auto const right =
    std::get_if<benson::ast::Postfix_expression>(&add->right->value);
  REQUIRE(right != nullptr);
  CHECK(spellings.lookup(right->op.spelling) == "^");
}

TEST_CASE("parse_expression - dereference: in full program")
{
  CHECK(parses("let f = fn(p: ^Int32): Int32 => { return p^; };"));
  CHECK(parses("let f = fn(p: ^[]Int32): Int32 => { return p^[0]; };"));
}

TEST_CASE("parse_expression - address-of")
{
  auto fixture = Parse_fixture{"&x"};
  auto const &spellings = fixture.spellings;
  auto const expr = fixture.parser.parse_expression();
  auto const unary = std::get_if<benson::ast::Prefix_expression>(&expr->value);
  REQUIRE(unary != nullptr);
  CHECK(spellings.lookup(unary->op.spelling) == "&");
  auto const operand =
    std::get_if<benson::ast::Identifier_expression>(&unary->operand->value);
  REQUIRE(operand != nullptr);
  CHECK(spellings.lookup(operand->identifier.spelling) == "x");
}

TEST_CASE("parse_expression - address-of: postfix binds tighter")
{
  // &a[0] should parse as &(a[0])
  auto fixture = Parse_fixture{"&a[0]"};
  auto const &spellings = fixture.spellings;
  auto const expr = fixture.parser.parse_expression();
  auto const unary = std::get_if<benson::ast::Prefix_expression>(&expr->value);
  REQUIRE(unary != nullptr);
  CHECK(spellings.lookup(unary->op.spelling) == "&");
  CHECK(
    std::get_if<benson::ast::Index_expression>(&unary->operand->value) !=
    nullptr
  );
}

TEST_CASE("parse_expression - address-of: in binary expression")
{
  // &a + &b should parse as (&a) + (&b)
  auto fixture = Parse_fixture{"&a + &b"};
  auto const &spellings = fixture.spellings;
  auto const expr = fixture.parser.parse_expression();
  auto const add = std::get_if<benson::ast::Binary_expression>(&expr->value);
  REQUIRE(add != nullptr);
  CHECK(spellings.lookup(add->op.spelling) == "+");
  auto const left =
    std::get_if<benson::ast::Prefix_expression>(&add->left->value);
  REQUIRE(left != nullptr);
  CHECK(spellings.lookup(left->op.spelling) == "&");
  auto const right =
    std::get_if<benson::ast::Prefix_expression>(&add->right->value);
  REQUIRE(right != nullptr);
  CHECK(spellings.lookup(right->op.spelling) == "&");
}

TEST_CASE("parse_expression - address-of: in full program")
{
  CHECK(parses("let f = fn(x: Int32): &Int32 => { return &x; };"));
}

TEST_CASE("parse_expression - if: simple")
{
  auto fixture = Parse_fixture{"if x { 1 }"};
  auto const &spellings = fixture.spellings;
  auto const expr = fixture.parser.parse_expression();
  auto const if_expr = std::get_if<benson::ast::If_expression>(&expr->value);
  REQUIRE(if_expr != nullptr);
  CHECK(spellings.lookup(if_expr->kw_if.spelling) == "if");
  auto const cond =
    std::get_if<benson::ast::Identifier_expression>(&if_expr->condition->value);
  REQUIRE(cond != nullptr);
  CHECK(spellings.lookup(cond->identifier.spelling) == "x");
  REQUIRE(if_expr->then_block.tail != nullptr);
  CHECK(if_expr->else_if_parts.empty());
  CHECK_FALSE(if_expr->else_part.has_value());
}

TEST_CASE("parse_expression - if else")
{
  auto fixture = Parse_fixture{"if x { 1 } else { 0 }"};
  auto const &spellings = fixture.spellings;
  auto const expr = fixture.parser.parse_expression();
  auto const if_expr = std::get_if<benson::ast::If_expression>(&expr->value);
  REQUIRE(if_expr != nullptr);
  CHECK(if_expr->else_if_parts.empty());
  REQUIRE(if_expr->else_part.has_value());
  CHECK(spellings.lookup(if_expr->else_part->kw_else.spelling) == "else");
  REQUIRE(if_expr->else_part->body.tail != nullptr);
}

TEST_CASE("parse_expression - else if chain")
{
  auto fixture = Parse_fixture{"if a { 1 } else if b { 2 } else { 3 }"};
  auto const &spellings = fixture.spellings;
  auto const expr = fixture.parser.parse_expression();
  auto const if_expr = std::get_if<benson::ast::If_expression>(&expr->value);
  REQUIRE(if_expr != nullptr);
  REQUIRE(if_expr->else_if_parts.size() == 1);
  CHECK(spellings.lookup(if_expr->else_if_parts[0].kw_else.spelling) == "else");
  CHECK(spellings.lookup(if_expr->else_if_parts[0].kw_if.spelling) == "if");
  auto const cond = std::get_if<benson::ast::Identifier_expression>(
    &if_expr->else_if_parts[0].condition->value
  );
  REQUIRE(cond != nullptr);
  CHECK(spellings.lookup(cond->identifier.spelling) == "b");
  REQUIRE(if_expr->else_if_parts[0].body.tail != nullptr);
  REQUIRE(if_expr->else_part.has_value());
  REQUIRE(if_expr->else_part->body.tail != nullptr);
}

TEST_CASE("parse_expression - if as expression statement")
{
  CHECK(parses("let f = fn(): Void => { if x { 1; }; };"));
}

TEST_CASE("parse_expression - if else as initializer")
{
  CHECK(parses("let f = fn(): Void => { let x = if cond { 1 } else { 0 }; };"));
}

TEST_CASE("parse_expression - if in full program")
{
  CHECK(parses(
    "let f = fn(x: Int32): Int32 => {"
    "  return if x { 1 } else { 0 };"
    "};"
  ));
  CHECK(parses(
    "let f = fn(x: Int32): Int32 => {"
    "  if x { return 1; };"
    "  return 0;"
    "};"
  ));
  CHECK(parses(
    "let f = fn(x: Int32): Int32 => {"
    "  return if x { 1 } else if y { 2 } else { 3 };"
    "};"
  ));
}

TEST_CASE("parse_expression - comparison operators")
{
  // each comparison operator parses correctly
  for (auto const op : {"<", "<=", ">", ">=", "==", "!="})
  {
    auto fixture = Parse_fixture{std::string{"1 "} + op + " 2"};
    auto const &spellings = fixture.spellings;
    auto const expr = fixture.parser.parse_expression();
    auto const bin = std::get_if<benson::ast::Binary_expression>(&expr->value);
    REQUIRE(bin != nullptr);
    CHECK(spellings.lookup(bin->op.spelling) == op);
    auto const left =
      std::get_if<benson::ast::Int_literal_expression>(&bin->left->value);
    REQUIRE(left != nullptr);
    CHECK(spellings.lookup(left->literal.spelling) == "1");
    auto const right =
      std::get_if<benson::ast::Int_literal_expression>(&bin->right->value);
    REQUIRE(right != nullptr);
    CHECK(spellings.lookup(right->literal.spelling) == "2");
  }
}

TEST_CASE("parse_expression - additive before comparison")
{
  // 1 + 2 < 3 + 4 should parse as (1 + 2) < (3 + 4)
  auto fixture = Parse_fixture{"1 + 2 < 3 + 4"};
  auto const &spellings = fixture.spellings;
  auto const expr = fixture.parser.parse_expression();
  auto const lt = std::get_if<benson::ast::Binary_expression>(&expr->value);
  REQUIRE(lt != nullptr);
  CHECK(spellings.lookup(lt->op.spelling) == "<");
  auto const left =
    std::get_if<benson::ast::Binary_expression>(&lt->left->value);
  REQUIRE(left != nullptr);
  CHECK(spellings.lookup(left->op.spelling) == "+");
  auto const right =
    std::get_if<benson::ast::Binary_expression>(&lt->right->value);
  REQUIRE(right != nullptr);
  CHECK(spellings.lookup(right->op.spelling) == "+");
}

TEST_CASE("parse_expression - comparison before equality")
{
  // 1 < 2 == 3 > 4 should parse as (1 < 2) == (3 > 4)
  auto fixture = Parse_fixture{"1 < 2 == 3 > 4"};
  auto const &spellings = fixture.spellings;
  auto const expr = fixture.parser.parse_expression();
  auto const eq = std::get_if<benson::ast::Binary_expression>(&expr->value);
  REQUIRE(eq != nullptr);
  CHECK(spellings.lookup(eq->op.spelling) == "==");
  auto const left =
    std::get_if<benson::ast::Binary_expression>(&eq->left->value);
  REQUIRE(left != nullptr);
  CHECK(spellings.lookup(left->op.spelling) == "<");
  auto const right =
    std::get_if<benson::ast::Binary_expression>(&eq->right->value);
  REQUIRE(right != nullptr);
  CHECK(spellings.lookup(right->op.spelling) == ">");
}

TEST_CASE("parse_expression - comparison in full program")
{
  CHECK(parses(
    "let f = fn(x: Int32): Int32 => { return if x < 10 { 0 } else { 1 }; };"
  ));
  CHECK(parses("let f = fn(a: Int32, b: Int32): Int32 => { return a == b; };"));
  CHECK(parses("let f = fn(a: Int32, b: Int32): Int32 => { return a != b; };"));
}

TEST_CASE("parse_expression - assign: simple")
{
  auto fixture = Parse_fixture{"x = 1"};
  auto const &spellings = fixture.spellings;
  auto const expr = fixture.parser.parse_expression();
  auto const bin = std::get_if<benson::ast::Binary_expression>(&expr->value);
  REQUIRE(bin != nullptr);
  CHECK(spellings.lookup(bin->op.spelling) == "=");
  auto const left =
    std::get_if<benson::ast::Identifier_expression>(&bin->left->value);
  REQUIRE(left != nullptr);
  CHECK(spellings.lookup(left->identifier.spelling) == "x");
  auto const right =
    std::get_if<benson::ast::Int_literal_expression>(&bin->right->value);
  REQUIRE(right != nullptr);
  CHECK(spellings.lookup(right->literal.spelling) == "1");
}

TEST_CASE("parse_expression - assign: right-associative")
{
  // a = b = 1 should parse as a = (b = 1)
  auto fixture = Parse_fixture{"a = b = 1"};
  auto const &spellings = fixture.spellings;
  auto const expr = fixture.parser.parse_expression();
  auto const outer = std::get_if<benson::ast::Binary_expression>(&expr->value);
  REQUIRE(outer != nullptr);
  CHECK(spellings.lookup(outer->op.spelling) == "=");
  auto const left =
    std::get_if<benson::ast::Identifier_expression>(&outer->left->value);
  REQUIRE(left != nullptr);
  CHECK(spellings.lookup(left->identifier.spelling) == "a");
  auto const inner =
    std::get_if<benson::ast::Binary_expression>(&outer->right->value);
  REQUIRE(inner != nullptr);
  CHECK(spellings.lookup(inner->op.spelling) == "=");
  auto const inner_left =
    std::get_if<benson::ast::Identifier_expression>(&inner->left->value);
  REQUIRE(inner_left != nullptr);
  CHECK(spellings.lookup(inner_left->identifier.spelling) == "b");
}

TEST_CASE("parse_expression - assign: lower precedence than equality")
{
  // x = a == b should parse as x = (a == b)
  auto fixture = Parse_fixture{"x = a == b"};
  auto const &spellings = fixture.spellings;
  auto const expr = fixture.parser.parse_expression();
  auto const assign = std::get_if<benson::ast::Binary_expression>(&expr->value);
  REQUIRE(assign != nullptr);
  CHECK(spellings.lookup(assign->op.spelling) == "=");
  auto const rhs =
    std::get_if<benson::ast::Binary_expression>(&assign->right->value);
  REQUIRE(rhs != nullptr);
  CHECK(spellings.lookup(rhs->op.spelling) == "==");
}

TEST_CASE("parse_expression - assign: in full program")
{
  CHECK(parses("let f = fn(mut x: Int32): Void => { x = 42; };"));
  CHECK(
    parses("let f = fn(mut x: Int32, mut y: Int32): Void => { x = y = 0; };")
  );
}

TEST_CASE("parse_statement - while: simple")
{
  auto fixture = Parse_fixture{"while x { }"};
  auto const &spellings = fixture.spellings;
  auto const stmt = fixture.parser.parse_statement();
  auto const while_stmt =
    std::get_if<benson::ast::While_statement>(&stmt.value);
  REQUIRE(while_stmt != nullptr);
  CHECK(spellings.lookup(while_stmt->kw_while.spelling) == "while");
  auto const cond = std::get_if<benson::ast::Identifier_expression>(
    &while_stmt->condition->value
  );
  REQUIRE(cond != nullptr);
  CHECK(spellings.lookup(cond->identifier.spelling) == "x");
  CHECK(while_stmt->body.statements.empty());
}

TEST_CASE("parse_statement - while: with body")
{
  auto fixture = Parse_fixture{"while n > 0 { f(n); }"};
  auto const &spellings = fixture.spellings;
  auto const stmt = fixture.parser.parse_statement();
  auto const while_stmt =
    std::get_if<benson::ast::While_statement>(&stmt.value);
  REQUIRE(while_stmt != nullptr);
  auto const cond =
    std::get_if<benson::ast::Binary_expression>(&while_stmt->condition->value);
  REQUIRE(cond != nullptr);
  CHECK(spellings.lookup(cond->op.spelling) == ">");
  CHECK(while_stmt->body.statements.size() == 1);
}

TEST_CASE("parse_statement - while: in full program")
{
  CHECK(parses(
    "let f = fn(mut n: Int32): Int32 => {"
    "  while n > 0 { n = n - 1; }"
    "  return n;"
    "};"
  ));
}

TEST_CASE("parse_let_statement - mut")
{
  auto fixture = Parse_fixture{"let mut x = 0;"};
  auto const &spellings = fixture.spellings;
  auto const stmt = fixture.parser.parse_statement();
  auto const let_stmt = std::get_if<benson::ast::Let_statement>(&stmt.value);
  REQUIRE(let_stmt != nullptr);
  CHECK(let_stmt->kw_mut.has_value());
  CHECK(spellings.lookup(let_stmt->kw_mut->spelling) == "mut");
  CHECK(spellings.lookup(let_stmt->name.spelling) == "x");
}

TEST_CASE("Parser - quicksort.benson parses successfully")
{
  auto file = std::ifstream{std::string{EXAMPLES_PATH} + "/quicksort.benson"};
  auto binary_stream = benson::Istream_binary_stream{&file};
  auto char_stream = benson::Utf8_char_stream{&binary_stream};
  auto spellings = benson::Spelling_table{};
  auto lexeme_stream = benson::Lexeme_stream{&char_stream, &spellings};
  auto reader = benson::Lexeme_stream_reader{&lexeme_stream};
  auto parser = benson::Parser{&reader, &spellings};
  auto const unit = parser.parse_translation_unit();
  CHECK(unit.statements.size() == 3);
}

TEST_CASE("parse_expression - prefix bracket unsized array type")
{
  auto fixture = Parse_fixture{"[]Int32"};
  auto const &spellings = fixture.spellings;
  auto const expr = fixture.parser.parse_expression();
  auto const prefix =
    std::get_if<benson::ast::Prefix_bracket_expression>(&expr->value);
  REQUIRE(prefix != nullptr);
  CHECK(spellings.lookup(prefix->lbracket.spelling) == "[");
  CHECK(prefix->size == nullptr);
  CHECK(spellings.lookup(prefix->rbracket.spelling) == "]");
  auto const operand =
    std::get_if<benson::ast::Identifier_expression>(&prefix->operand->value);
  REQUIRE(operand != nullptr);
  CHECK(spellings.lookup(operand->identifier.spelling) == "Int32");
}

TEST_CASE("parse_expression - prefix bracket sized array type")
{
  auto fixture = Parse_fixture{"[4]Int32"};
  auto const &spellings = fixture.spellings;
  auto const expr = fixture.parser.parse_expression();
  auto const prefix =
    std::get_if<benson::ast::Prefix_bracket_expression>(&expr->value);
  REQUIRE(prefix != nullptr);
  CHECK(spellings.lookup(prefix->lbracket.spelling) == "[");
  REQUIRE(prefix->size != nullptr);
  auto const size =
    std::get_if<benson::ast::Int_literal_expression>(&prefix->size->value);
  REQUIRE(size != nullptr);
  CHECK(spellings.lookup(size->literal.spelling) == "4");
  CHECK(spellings.lookup(prefix->rbracket.spelling) == "]");
  auto const operand =
    std::get_if<benson::ast::Identifier_expression>(&prefix->operand->value);
  REQUIRE(operand != nullptr);
  CHECK(spellings.lookup(operand->identifier.spelling) == "Int32");
}

TEST_CASE("parse_expression - pointer to unsized array type")
{
  auto fixture = Parse_fixture{"^[]Int32"};
  auto const &spellings = fixture.spellings;
  auto const expr = fixture.parser.parse_expression();
  auto const unary = std::get_if<benson::ast::Prefix_expression>(&expr->value);
  REQUIRE(unary != nullptr);
  CHECK(spellings.lookup(unary->op.spelling) == "^");
  auto const prefix =
    std::get_if<benson::ast::Prefix_bracket_expression>(&unary->operand->value);
  REQUIRE(prefix != nullptr);
  CHECK(prefix->size == nullptr);
  auto const operand =
    std::get_if<benson::ast::Identifier_expression>(&prefix->operand->value);
  REQUIRE(operand != nullptr);
  CHECK(spellings.lookup(operand->identifier.spelling) == "Int32");
}

TEST_CASE("parse_expression - ^mut prefix")
{
  auto fixture = Parse_fixture{"^mut Int32"};
  auto const &spellings = fixture.spellings;
  auto const expr = fixture.parser.parse_expression();
  auto const unary = std::get_if<benson::ast::Prefix_expression>(&expr->value);
  REQUIRE(unary != nullptr);
  CHECK(spellings.lookup(unary->op.spelling) == "^mut");
  CHECK(unary->op.token == benson::Token::caret_mut);
  auto const operand =
    std::get_if<benson::ast::Identifier_expression>(&unary->operand->value);
  REQUIRE(operand != nullptr);
  CHECK(spellings.lookup(operand->identifier.spelling) == "Int32");
}

TEST_CASE("parse_expression - ^mut []Int32 as mutable pointer to unsized array")
{
  auto fixture = Parse_fixture{"^mut []Int32"};
  auto const &spellings = fixture.spellings;
  auto const expr = fixture.parser.parse_expression();
  auto const unary = std::get_if<benson::ast::Prefix_expression>(&expr->value);
  REQUIRE(unary != nullptr);
  CHECK(unary->op.token == benson::Token::caret_mut);
  auto const prefix =
    std::get_if<benson::ast::Prefix_bracket_expression>(&unary->operand->value);
  REQUIRE(prefix != nullptr);
  CHECK(prefix->size == nullptr);
  auto const operand =
    std::get_if<benson::ast::Identifier_expression>(&prefix->operand->value);
  REQUIRE(operand != nullptr);
  CHECK(spellings.lookup(operand->identifier.spelling) == "Int32");
}

TEST_CASE("Parser - accepts prefix type syntax")
{
  CHECK(parses("let f = fn(x: []Int32): Void => { };"));
  CHECK(parses("let f = fn(x: [4]Int32): Void => { };"));
  CHECK(parses("let f = fn(x: ^Int32): Void => { };"));
  CHECK(parses("let f = fn(x: ^mut Int32): Void => { };"));
  CHECK(parses("let f = fn(x: ^mut []Int32): Void => { };"));
  CHECK(parses("let f = fn(x: ^[]Int32): Void => { };"));
  CHECK(parses("let f = fn(x: [][4]Int32): Void => { };"));
}

TEST_CASE("parse_fn_expression - named function expression")
{
  auto fixture = Parse_fixture{"fn main(): Int32 => { return 0; };"};
  auto const &spellings = fixture.spellings;
  auto const stmt = fixture.parser.parse_expression_statement();
  auto const fn =
    std::get_if<benson::ast::Fn_expression>(&stmt.expression.value);
  REQUIRE(fn != nullptr);
  CHECK(spellings.lookup(fn->kw_fn.spelling) == "fn");
  REQUIRE(fn->name.has_value());
  CHECK(spellings.lookup(fn->name->spelling) == "main");
  CHECK(fn->parameters.empty());
  auto const ret_type = std::get_if<benson::ast::Identifier_expression>(
    &fn->return_type_specifier.type->value
  );
  REQUIRE(ret_type != nullptr);
  CHECK(spellings.lookup(ret_type->identifier.spelling) == "Int32");
}

TEST_CASE("parse_fn_expression - anonymous function has no name")
{
  auto fixture = Parse_fixture{"fn(): Int32 => 0"};
  auto const expr = fixture.parser.parse_expression();
  auto const fn = std::get_if<benson::ast::Fn_expression>(&expr->value);
  REQUIRE(fn != nullptr);
  CHECK_FALSE(fn->name.has_value());
}

TEST_CASE("parse_translation_unit - named fn statement")
{
  auto fixture = Parse_fixture{"fn add(x: Int32, y: Int32): Int32 => x + y;"};
  auto const &spellings = fixture.spellings;
  auto const unit = fixture.parser.parse_translation_unit();
  REQUIRE(unit.statements.size() == 1);
  auto const &stmt =
    std::get<benson::ast::Expression_statement>(unit.statements[0].value);
  auto const fn =
    std::get_if<benson::ast::Fn_expression>(&stmt.expression.value);
  REQUIRE(fn != nullptr);
  REQUIRE(fn->name.has_value());
  CHECK(spellings.lookup(fn->name->spelling) == "add");
  REQUIRE(fn->parameters.size() == 2);
  CHECK(spellings.lookup(fn->parameters[0].name.spelling) == "x");
  CHECK(spellings.lookup(fn->parameters[1].name.spelling) == "y");
}
