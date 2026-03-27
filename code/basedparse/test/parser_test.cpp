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
  REQUIRE(unit->function_definitions.size() == 1);
  auto const &fn_def = unit->function_definitions[0];
  CHECK(fn_def.kw_let.text == "let");
  CHECK(fn_def.name.text == "main");
  CHECK(fn_def.eq.text == "=");
  CHECK(fn_def.function.kw_fn.text == "fn");
  CHECK(fn_def.function.lparen.text == "(");
  CHECK(fn_def.function.rparen.text == ")");
  REQUIRE(fn_def.function.return_type_specifier.has_value());
  CHECK(fn_def.function.return_type_specifier->colon.text == ":");
  auto const return_type = std::get_if<basedparse::Identifier_expression>(
    &fn_def.function.return_type_specifier->type->value
  );
  REQUIRE(return_type != nullptr);
  CHECK(return_type->identifier.text == "Int32");
  CHECK(fn_def.function.arrow.text == "=>");
  REQUIRE(fn_def.function.body != nullptr);
  auto const body =
    std::get_if<basedparse::Block_expression>(&fn_def.function.body->value);
  REQUIRE(body != nullptr);
  CHECK(body->lbrace.text == "{");
  REQUIRE(body->statements.size() == 1);
  auto const ret_stmt =
    std::get_if<basedparse::Return_statement>(&body->statements[0].value);
  REQUIRE(ret_stmt != nullptr);
  CHECK(ret_stmt->kw_return.text == "return");
  auto const int_lit =
    std::get_if<basedparse::Int_literal_expression>(&ret_stmt->value.value);
  REQUIRE(int_lit != nullptr);
  CHECK(int_lit->literal.text == "0");
  CHECK(ret_stmt->semicolon.text == ";");
  CHECK(body->rbrace.text == "}");
}

TEST_CASE("Parser - parameters.based parses successfully")
{
  auto file = std::ifstream{std::string{EXAMPLES_PATH} + "/parameters.based"};
  auto binary_stream = basedlex::Istream_binary_stream{&file};
  auto char_stream = basedlex::Utf8_char_stream{&binary_stream};
  auto lexeme_stream = basedlex::Lexeme_stream{&char_stream};
  auto reader = basedlex::Lexeme_stream_reader{&lexeme_stream};
  auto parser = basedparse::Parser{&reader};
  auto const unit = parser.parse_translation_unit();
  REQUIRE(unit->function_definitions.size() == 3);
  auto const &id_def = unit->function_definitions[0];
  auto const &first_def = unit->function_definitions[1];
  auto const &main_def = unit->function_definitions[2];
  CHECK(id_def.name.text == "id");
  CHECK(first_def.name.text == "first");
  CHECK(main_def.name.text == "main");
  // id: fn(x: Int32): Int32 => { return x; }
  REQUIRE(id_def.function.parameters.size() == 1);
  CHECK(id_def.function.parameters[0].name.text == "x");
  auto const id_param_type = std::get_if<basedparse::Identifier_expression>(
    &id_def.function.parameters[0].type->value
  );
  REQUIRE(id_param_type != nullptr);
  CHECK(id_param_type->identifier.text == "Int32");
  REQUIRE(id_def.function.return_type_specifier.has_value());
  auto const id_ret_type = std::get_if<basedparse::Identifier_expression>(
    &id_def.function.return_type_specifier->type->value
  );
  REQUIRE(id_ret_type != nullptr);
  CHECK(id_ret_type->identifier.text == "Int32");
  auto const id_body =
    std::get_if<basedparse::Block_expression>(&id_def.function.body->value);
  REQUIRE(id_body != nullptr);
  REQUIRE(id_body->statements.size() == 1);
  auto const id_ret =
    std::get_if<basedparse::Return_statement>(&id_body->statements[0].value);
  REQUIRE(id_ret != nullptr);
  auto const id_ret_val =
    std::get_if<basedparse::Identifier_expression>(&id_ret->value.value);
  REQUIRE(id_ret_val != nullptr);
  CHECK(id_ret_val->identifier.text == "x");
  // first: fn(x: Int32, y: Int32): Int32 => { return x; }
  REQUIRE(first_def.function.parameters.size() == 2);
  CHECK(first_def.function.parameters[0].name.text == "x");
  auto const first_param0_type = std::get_if<basedparse::Identifier_expression>(
    &first_def.function.parameters[0].type->value
  );
  REQUIRE(first_param0_type != nullptr);
  CHECK(first_param0_type->identifier.text == "Int32");
  CHECK(first_def.function.parameters[1].name.text == "y");
  auto const first_param1_type = std::get_if<basedparse::Identifier_expression>(
    &first_def.function.parameters[1].type->value
  );
  REQUIRE(first_param1_type != nullptr);
  CHECK(first_param1_type->identifier.text == "Int32");
  REQUIRE(first_def.function.return_type_specifier.has_value());
  auto const first_ret_type = std::get_if<basedparse::Identifier_expression>(
    &first_def.function.return_type_specifier->type->value
  );
  REQUIRE(first_ret_type != nullptr);
  CHECK(first_ret_type->identifier.text == "Int32");
  auto const first_body =
    std::get_if<basedparse::Block_expression>(&first_def.function.body->value);
  REQUIRE(first_body != nullptr);
  REQUIRE(first_body->statements.size() == 1);
  auto const first_ret =
    std::get_if<basedparse::Return_statement>(&first_body->statements[0].value);
  REQUIRE(first_ret != nullptr);
  auto const first_ret_val =
    std::get_if<basedparse::Identifier_expression>(&first_ret->value.value);
  REQUIRE(first_ret_val != nullptr);
  CHECK(first_ret_val->identifier.text == "x");
  // main: fn(): Int32 => { return first(id(42), 0); }
  REQUIRE(main_def.function.return_type_specifier.has_value());
  auto const main_ret_type = std::get_if<basedparse::Identifier_expression>(
    &main_def.function.return_type_specifier->type->value
  );
  REQUIRE(main_ret_type != nullptr);
  CHECK(main_ret_type->identifier.text == "Int32");
  auto const main_body =
    std::get_if<basedparse::Block_expression>(&main_def.function.body->value);
  REQUIRE(main_body != nullptr);
  REQUIRE(main_body->statements.size() == 1);
  auto const main_ret =
    std::get_if<basedparse::Return_statement>(&main_body->statements[0].value);
  REQUIRE(main_ret != nullptr);
  // first(id(42), 0) — outer call
  auto const outer_call =
    std::get_if<basedparse::Call_expression>(&main_ret->value.value);
  REQUIRE(outer_call != nullptr);
  auto const outer_callee =
    std::get_if<basedparse::Identifier_expression>(&outer_call->callee->value);
  REQUIRE(outer_callee != nullptr);
  CHECK(outer_callee->identifier.text == "first");
  REQUIRE(outer_call->arguments.size() == 2);
  auto const inner_call =
    std::get_if<basedparse::Call_expression>(&outer_call->arguments[0].value);
  REQUIRE(inner_call != nullptr);
  auto const inner_callee =
    std::get_if<basedparse::Identifier_expression>(&inner_call->callee->value);
  REQUIRE(inner_callee != nullptr);
  CHECK(inner_callee->identifier.text == "id");
  REQUIRE(inner_call->arguments.size() == 1);
  auto const inner_arg = std::get_if<basedparse::Int_literal_expression>(
    &inner_call->arguments[0].value
  );
  REQUIRE(inner_arg != nullptr);
  CHECK(inner_arg->literal.text == "42");
  auto const outer_arg1 = std::get_if<basedparse::Int_literal_expression>(
    &outer_call->arguments[1].value
  );
  REQUIRE(outer_arg1 != nullptr);
  CHECK(outer_arg1->literal.text == "0");
}

TEST_CASE("Parser - call_expression.based parses successfully")
{
  auto file =
    std::ifstream{std::string{EXAMPLES_PATH} + "/call_expression.based"};
  auto binary_stream = basedlex::Istream_binary_stream{&file};
  auto char_stream = basedlex::Utf8_char_stream{&binary_stream};
  auto lexeme_stream = basedlex::Lexeme_stream{&char_stream};
  auto reader = basedlex::Lexeme_stream_reader{&lexeme_stream};
  auto parser = basedparse::Parser{&reader};
  auto const unit = parser.parse_translation_unit();
  REQUIRE(unit->function_definitions.size() == 2);
  auto const &foo = unit->function_definitions[0];
  auto const &main = unit->function_definitions[1];
  CHECK(foo.name.text == "foo");
  CHECK(main.name.text == "main");
  // foo returns (fn(): Int32 => { return 0; })() — a call with a paren-wrapped
  // fn expression callee
  auto const foo_body =
    std::get_if<basedparse::Block_expression>(&foo.function.body->value);
  REQUIRE(foo_body != nullptr);
  REQUIRE(foo_body->statements.size() == 1);
  auto const foo_ret =
    std::get_if<basedparse::Return_statement>(&foo_body->statements[0].value);
  REQUIRE(foo_ret != nullptr);
  auto const foo_call =
    std::get_if<basedparse::Call_expression>(&foo_ret->value.value);
  REQUIRE(foo_call != nullptr);
  CHECK(foo_call->lparen.text == "(");
  CHECK(foo_call->rparen.text == ")");
  auto const foo_paren =
    std::get_if<basedparse::Paren_expression>(&foo_call->callee->value);
  REQUIRE(foo_paren != nullptr);
  CHECK(
    std::get_if<basedparse::Fn_expression>(&foo_paren->inner->value) != nullptr
  );
  // main returns foo() — a call with an identifier callee
  auto const main_body =
    std::get_if<basedparse::Block_expression>(&main.function.body->value);
  REQUIRE(main_body != nullptr);
  REQUIRE(main_body->statements.size() == 1);
  auto const main_ret =
    std::get_if<basedparse::Return_statement>(&main_body->statements[0].value);
  REQUIRE(main_ret != nullptr);
  auto const main_call =
    std::get_if<basedparse::Call_expression>(&main_ret->value.value);
  REQUIRE(main_call != nullptr);
  CHECK(main_call->lparen.text == "(");
  CHECK(main_call->rparen.text == ")");
  auto const callee =
    std::get_if<basedparse::Identifier_expression>(&main_call->callee->value);
  REQUIRE(callee != nullptr);
  CHECK(callee->identifier.text == "foo");
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
}

TEST_CASE("Parser - rejects invalid code")
{
  CHECK_FALSE(parses("let"));
  CHECK_FALSE(parses("let x"));
  CHECK_FALSE(parses("let x ="));
  CHECK_FALSE(parses("let x = 42;"));
  CHECK_FALSE(parses("let x = fn()"));
  CHECK_FALSE(parses("let x = fn() =>"));
  CHECK_FALSE(parses("let x = fn() => Int32"));
  CHECK_FALSE(parses("let x = fn(): Int32 => {"));
  CHECK_FALSE(parses("let x = fn(): Int32 => { }"));
  CHECK_FALSE(parses("return 0;"));
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
  CHECK(parses("let x = fn(): Int32 => { return * 1; };"));
  CHECK(parses("let x = fn(): Int32 => { return 1 + * 2; };"));
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
  CHECK(unit->function_definitions.empty());
}

TEST_CASE("parse_translation_unit - multiple functions")
{
  auto fixture = Parse_fixture{"let a = fn(): Int32 => { return 1; };\n"
                               "let b = fn(): Int32 => { return 2; };"};
  auto const unit = fixture.parser.parse_translation_unit();
  REQUIRE(unit->function_definitions.size() == 2);
  CHECK(unit->function_definitions[0].name.text == "a");
  CHECK(unit->function_definitions[1].name.text == "b");
}

TEST_CASE("parse_function_definition")
{
  auto fixture = Parse_fixture{"let main = fn(): Int32 => { return 0; };"};
  auto const fn_def = fixture.parser.parse_function_definition();
  CHECK(fn_def.kw_let.text == "let");
  CHECK(fn_def.name.text == "main");
  CHECK(fn_def.eq.text == "=");
  CHECK(fn_def.function.kw_fn.text == "fn");
  REQUIRE(fn_def.function.body != nullptr);
  auto const body =
    std::get_if<basedparse::Block_expression>(&fn_def.function.body->value);
  REQUIRE(body != nullptr);
  CHECK(body->statements.size() == 1);
}

TEST_CASE("parse_let_statement")
{
  auto fixture = Parse_fixture{"let x = 42;"};
  auto const stmt = fixture.parser.parse_let_statement();
  CHECK(stmt.kw_let.text == "let");
  CHECK(stmt.name.text == "x");
  CHECK(stmt.eq.text == "=");
  auto const lit =
    std::get_if<basedparse::Int_literal_expression>(&stmt.initializer.value);
  REQUIRE(lit != nullptr);
  CHECK(lit->literal.text == "42");
  CHECK(stmt.semicolon.text == ";");
}

TEST_CASE("parse_return_statement")
{
  auto fixture = Parse_fixture{"return 99;"};
  auto const stmt = fixture.parser.parse_return_statement();
  CHECK(stmt.kw_return.text == "return");
  auto const lit =
    std::get_if<basedparse::Int_literal_expression>(&stmt.value.value);
  REQUIRE(lit != nullptr);
  CHECK(lit->literal.text == "99");
  CHECK(stmt.semicolon.text == ";");
}

TEST_CASE("parse_expression_statement")
{
  auto fixture = Parse_fixture{"foo;"};
  auto const stmt = fixture.parser.parse_expression_statement();
  auto const id =
    std::get_if<basedparse::Identifier_expression>(&stmt.expression.value);
  REQUIRE(id != nullptr);
  CHECK(id->identifier.text == "foo");
  CHECK(stmt.semicolon.text == ";");
}

TEST_CASE("parse_block_expression")
{
  auto fixture = Parse_fixture{"{ return 1; let x = 2; }"};
  auto const block = fixture.parser.parse_block_expression();
  CHECK(block.lbrace.text == "{");
  REQUIRE(block.statements.size() == 2);
  CHECK(
    std::get_if<basedparse::Return_statement>(&block.statements[0].value) !=
    nullptr
  );
  CHECK(
    std::get_if<basedparse::Let_statement>(&block.statements[1].value) !=
    nullptr
  );
  CHECK(block.rbrace.text == "}");
}

TEST_CASE("parse_block_expression - empty")
{
  auto fixture = Parse_fixture{"{ }"};
  auto const block = fixture.parser.parse_block_expression();
  CHECK(block.lbrace.text == "{");
  CHECK(block.statements.empty());
  CHECK(block.tail == nullptr);
  CHECK(block.rbrace.text == "}");
}

TEST_CASE("parse_block_expression - tail expression")
{
  auto fixture = Parse_fixture{"{ 42 }"};
  auto const block = fixture.parser.parse_block_expression();
  CHECK(block.statements.empty());
  REQUIRE(block.tail != nullptr);
  auto const lit =
    std::get_if<basedparse::Int_literal_expression>(&block.tail->value);
  REQUIRE(lit != nullptr);
  CHECK(lit->literal.text == "42");
}

TEST_CASE("parse_block_expression - statements then tail")
{
  auto fixture = Parse_fixture{"{ let x = 1; x + 1 }"};
  auto const block = fixture.parser.parse_block_expression();
  REQUIRE(block.statements.size() == 1);
  CHECK(
    std::get_if<basedparse::Let_statement>(&block.statements[0].value) !=
    nullptr
  );
  REQUIRE(block.tail != nullptr);
  auto const bin =
    std::get_if<basedparse::Binary_expression>(&block.tail->value);
  REQUIRE(bin != nullptr);
  CHECK(bin->op.text == "+");
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
  auto const block = std::get_if<basedparse::Block_expression>(&expr->value);
  REQUIRE(block != nullptr);
  REQUIRE(block->tail != nullptr);
  CHECK(
    std::get_if<basedparse::Binary_expression>(&block->tail->value) != nullptr
  );
}

TEST_CASE("parse_expression - nested blocks")
{
  auto fixture = Parse_fixture{"{{{{}}}}"};
  auto const expr = fixture.parser.parse_expression();
  // outer block: no statements, tail is a block
  auto const b0 = std::get_if<basedparse::Block_expression>(&expr->value);
  REQUIRE(b0 != nullptr);
  CHECK(b0->statements.empty());
  REQUIRE(b0->tail != nullptr);
  auto const b1 = std::get_if<basedparse::Block_expression>(&b0->tail->value);
  REQUIRE(b1 != nullptr);
  CHECK(b1->statements.empty());
  REQUIRE(b1->tail != nullptr);
  auto const b2 = std::get_if<basedparse::Block_expression>(&b1->tail->value);
  REQUIRE(b2 != nullptr);
  CHECK(b2->statements.empty());
  REQUIRE(b2->tail != nullptr);
  auto const b3 = std::get_if<basedparse::Block_expression>(&b2->tail->value);
  REQUIRE(b3 != nullptr);
  CHECK(b3->statements.empty());
  CHECK(b3->tail == nullptr);
}

TEST_CASE("parse_expression - block as expression statement")
{
  CHECK(parses("let f = fn() => { { 42 }; };"));
}

TEST_CASE("parse_expression - block as initializer")
{
  CHECK(parses("let f = fn() => { let x = { let a = 1; a + 1 }; };"));
}

TEST_CASE("parse_block_expression - fn body parses tail syntactically")
{
  auto fixture = Parse_fixture{"fn(): Int32 => { 42 }"};
  auto const fn = fixture.parser.parse_fn_expression();
  REQUIRE(fn.body != nullptr);
  auto const body = std::get_if<basedparse::Block_expression>(&fn.body->value);
  REQUIRE(body != nullptr);
  CHECK(body->statements.empty());
  REQUIRE(body->tail != nullptr);
  auto const lit =
    std::get_if<basedparse::Int_literal_expression>(&body->tail->value);
  REQUIRE(lit != nullptr);
  CHECK(lit->literal.text == "42");
}

TEST_CASE("parse_fn_expression")
{
  auto fixture = Parse_fixture{"fn(): Int32 => { return 0; }"};
  auto const fn = fixture.parser.parse_fn_expression();
  CHECK(fn.kw_fn.text == "fn");
  CHECK(fn.lparen.text == "(");
  CHECK(fn.rparen.text == ")");
  REQUIRE(fn.return_type_specifier.has_value());
  CHECK(fn.return_type_specifier->colon.text == ":");
  auto const ret_type = std::get_if<basedparse::Identifier_expression>(
    &fn.return_type_specifier->type->value
  );
  REQUIRE(ret_type != nullptr);
  CHECK(ret_type->identifier.text == "Int32");
  CHECK(fn.arrow.text == "=>");
  REQUIRE(fn.body != nullptr);
  auto const body = std::get_if<basedparse::Block_expression>(&fn.body->value);
  REQUIRE(body != nullptr);
  CHECK(body->statements.size() == 1);
}

TEST_CASE("parse_fn_expression - mut parameter")
{
  auto fixture = Parse_fixture{"fn(mut x: Int32) => { }"};
  auto const fn = fixture.parser.parse_fn_expression();
  REQUIRE(fn.parameters.size() == 1);
  REQUIRE(fn.parameters[0].kw_mut.has_value());
  CHECK(fn.parameters[0].kw_mut->text == "mut");
  CHECK(fn.parameters[0].name.text == "x");
}

TEST_CASE("parse_fn_expression - mixed mut and non-mut parameters")
{
  auto fixture = Parse_fixture{"fn(x: Int32, mut y: Int32) => { }"};
  auto const fn = fixture.parser.parse_fn_expression();
  REQUIRE(fn.parameters.size() == 2);
  CHECK_FALSE(fn.parameters[0].kw_mut.has_value());
  CHECK(fn.parameters[0].name.text == "x");
  REQUIRE(fn.parameters[1].kw_mut.has_value());
  CHECK(fn.parameters[1].name.text == "y");
}

TEST_CASE("parse_fn_expression - no return type")
{
  auto fixture = Parse_fixture{"fn() => { }"};
  auto const fn = fixture.parser.parse_fn_expression();
  CHECK_FALSE(fn.return_type_specifier.has_value());
  REQUIRE(fn.body != nullptr);
  auto const body = std::get_if<basedparse::Block_expression>(&fn.body->value);
  REQUIRE(body != nullptr);
  CHECK(body->statements.empty());
}

TEST_CASE("parse_int_literal_expression")
{
  auto fixture = Parse_fixture{"42"};
  auto const expr = fixture.parser.parse_int_literal_expression();
  CHECK(expr.literal.text == "42");
}

TEST_CASE("parse_identifier_expression")
{
  auto fixture = Parse_fixture{"foo"};
  auto const expr = fixture.parser.parse_identifier_expression();
  CHECK(expr.identifier.text == "foo");
}

TEST_CASE("parse_paren_expression")
{
  auto fixture = Parse_fixture{"(42)"};
  auto const expr = fixture.parser.parse_paren_expression();
  CHECK(expr.lparen.text == "(");
  auto const inner =
    std::get_if<basedparse::Int_literal_expression>(&expr.inner->value);
  REQUIRE(inner != nullptr);
  CHECK(inner->literal.text == "42");
  CHECK(expr.rparen.text == ")");
}

TEST_CASE("parse_paren_expression - nested")
{
  auto fixture = Parse_fixture{"((x))"};
  auto const expr = fixture.parser.parse_paren_expression();
  CHECK(expr.lparen.text == "(");
  auto const inner =
    std::get_if<basedparse::Paren_expression>(&expr.inner->value);
  REQUIRE(inner != nullptr);
  auto const id =
    std::get_if<basedparse::Identifier_expression>(&inner->inner->value);
  REQUIRE(id != nullptr);
  CHECK(id->identifier.text == "x");
  CHECK(inner->rparen.text == ")");
  CHECK(expr.rparen.text == ")");
}

TEST_CASE("parse_primary_expression - dispatches to int literal")
{
  auto fixture = Parse_fixture{"123"};
  auto const expr = fixture.parser.parse_primary_expression();
  CHECK(
    std::get_if<basedparse::Int_literal_expression>(&expr->value) != nullptr
  );
}

TEST_CASE("parse_primary_expression - dispatches to identifier")
{
  auto fixture = Parse_fixture{"x"};
  auto const expr = fixture.parser.parse_primary_expression();
  CHECK(
    std::get_if<basedparse::Identifier_expression>(&expr->value) != nullptr
  );
}

TEST_CASE("parse_primary_expression - dispatches to fn")
{
  auto fixture = Parse_fixture{"fn(): Int32 => { }"};
  auto const expr = fixture.parser.parse_primary_expression();
  CHECK(std::get_if<basedparse::Fn_expression>(&expr->value) != nullptr);
}

TEST_CASE("parse_primary_expression - dispatches to paren")
{
  auto fixture = Parse_fixture{"(42)"};
  auto const expr = fixture.parser.parse_primary_expression();
  CHECK(std::get_if<basedparse::Paren_expression>(&expr->value) != nullptr);
}

TEST_CASE("parse_expression - simple binary expression")
{
  auto fixture = Parse_fixture{"1 + 2"};
  auto const expr = fixture.parser.parse_expression();
  auto const bin = std::get_if<basedparse::Binary_expression>(&expr->value);
  REQUIRE(bin != nullptr);
  auto const left =
    std::get_if<basedparse::Int_literal_expression>(&bin->left->value);
  REQUIRE(left != nullptr);
  CHECK(left->literal.text == "1");
  CHECK(bin->op.text == "+");
  auto const right =
    std::get_if<basedparse::Int_literal_expression>(&bin->right->value);
  REQUIRE(right != nullptr);
  CHECK(right->literal.text == "2");
}

TEST_CASE("parse_expression - multiplicative before additive")
{
  // 1 + 2 * 3 should parse as 1 + (2 * 3)
  auto fixture = Parse_fixture{"1 + 2 * 3"};
  auto const expr = fixture.parser.parse_expression();
  auto const add = std::get_if<basedparse::Binary_expression>(&expr->value);
  REQUIRE(add != nullptr);
  CHECK(add->op.text == "+");
  auto const mul =
    std::get_if<basedparse::Binary_expression>(&add->right->value);
  REQUIRE(mul != nullptr);
  CHECK(mul->op.text == "*");
  auto const two =
    std::get_if<basedparse::Int_literal_expression>(&mul->left->value);
  REQUIRE(two != nullptr);
  CHECK(two->literal.text == "2");
  auto const three =
    std::get_if<basedparse::Int_literal_expression>(&mul->right->value);
  REQUIRE(three != nullptr);
  CHECK(three->literal.text == "3");
}

TEST_CASE("parse_expression - left associativity")
{
  // 1 - 2 - 3 should parse as (1 - 2) - 3
  auto fixture = Parse_fixture{"1 - 2 - 3"};
  auto const expr = fixture.parser.parse_expression();
  auto const outer = std::get_if<basedparse::Binary_expression>(&expr->value);
  REQUIRE(outer != nullptr);
  CHECK(outer->op.text == "-");
  auto const inner =
    std::get_if<basedparse::Binary_expression>(&outer->left->value);
  REQUIRE(inner != nullptr);
  CHECK(inner->op.text == "-");
  auto const three =
    std::get_if<basedparse::Int_literal_expression>(&outer->right->value);
  REQUIRE(three != nullptr);
  CHECK(three->literal.text == "3");
}

TEST_CASE("parse_expression - all operators")
{
  // -f() + +2 - 3 * 4 / 5 % 6
  // parses as (-(f()) + (+2)) - (((3 * 4) / 5) % 6)
  auto fixture = Parse_fixture{"-f() + +2 - 3 * 4 / 5 % 6"};
  auto const expr = fixture.parser.parse_expression();
  // outer: (-f() + +2) - (...)
  auto const sub = std::get_if<basedparse::Binary_expression>(&expr->value);
  REQUIRE(sub != nullptr);
  CHECK(sub->op.text == "-");
  // left of -: -f() + +2
  auto const add =
    std::get_if<basedparse::Binary_expression>(&sub->left->value);
  REQUIRE(add != nullptr);
  CHECK(add->op.text == "+");
  // left of +: -f()
  auto const unary_minus =
    std::get_if<basedparse::Unary_expression>(&add->left->value);
  REQUIRE(unary_minus != nullptr);
  CHECK(unary_minus->op.text == "-");
  auto const call =
    std::get_if<basedparse::Call_expression>(&unary_minus->operand->value);
  REQUIRE(call != nullptr);
  auto const callee =
    std::get_if<basedparse::Identifier_expression>(&call->callee->value);
  REQUIRE(callee != nullptr);
  CHECK(callee->identifier.text == "f");
  // right of +: +2
  auto const unary_plus =
    std::get_if<basedparse::Unary_expression>(&add->right->value);
  REQUIRE(unary_plus != nullptr);
  CHECK(unary_plus->op.text == "+");
  // right of -: ((3 * 4) / 5) % 6
  auto const mod =
    std::get_if<basedparse::Binary_expression>(&sub->right->value);
  REQUIRE(mod != nullptr);
  CHECK(mod->op.text == "%");
  // left of %: (3 * 4) / 5
  auto const div =
    std::get_if<basedparse::Binary_expression>(&mod->left->value);
  REQUIRE(div != nullptr);
  CHECK(div->op.text == "/");
  // left of /: 3 * 4
  auto const mul =
    std::get_if<basedparse::Binary_expression>(&div->left->value);
  REQUIRE(mul != nullptr);
  CHECK(mul->op.text == "*");
}

TEST_CASE("parse_expression - call binds tighter than binary op")
{
  // f() + 1 should parse as (f()) + 1
  auto fixture = Parse_fixture{"f() + 1"};
  auto const expr = fixture.parser.parse_expression();
  auto const add = std::get_if<basedparse::Binary_expression>(&expr->value);
  REQUIRE(add != nullptr);
  CHECK(add->op.text == "+");
  CHECK(std::get_if<basedparse::Call_expression>(&add->left->value) != nullptr);
}

TEST_CASE("parse_statement - dispatches to let")
{
  auto fixture = Parse_fixture{"let x = 1;"};
  auto const stmt = fixture.parser.parse_statement();
  CHECK(std::get_if<basedparse::Let_statement>(&stmt.value) != nullptr);
}

TEST_CASE("parse_statement - dispatches to return")
{
  auto fixture = Parse_fixture{"return 1;"};
  auto const stmt = fixture.parser.parse_statement();
  CHECK(std::get_if<basedparse::Return_statement>(&stmt.value) != nullptr);
}

TEST_CASE("parse_statement - dispatches to expression statement")
{
  auto fixture = Parse_fixture{"x;"};
  auto const stmt = fixture.parser.parse_statement();
  CHECK(std::get_if<basedparse::Expression_statement>(&stmt.value) != nullptr);
}

TEST_CASE("parse_expression - unary minus: call binds tighter")
{
  // -f() should parse as -(f()), not (-f)()
  auto fixture = Parse_fixture{"-f()"};
  auto const expr = fixture.parser.parse_expression();
  auto const unary = std::get_if<basedparse::Unary_expression>(&expr->value);
  REQUIRE(unary != nullptr);
  CHECK(unary->op.text == "-");
  CHECK(
    std::get_if<basedparse::Call_expression>(&unary->operand->value) != nullptr
  );
}

TEST_CASE("parse_fn_expression - array parameter")
{
  auto fixture = Parse_fixture{"fn(buf: Int32[4]) => { }"};
  auto const fn = fixture.parser.parse_fn_expression();
  REQUIRE(fn.parameters.size() == 1);
  CHECK(fn.parameters[0].name.text == "buf");
  auto const param_type =
    std::get_if<basedparse::Index_expression>(&fn.parameters[0].type->value);
  REQUIRE(param_type != nullptr);
  auto const elem =
    std::get_if<basedparse::Identifier_expression>(&param_type->operand->value);
  REQUIRE(elem != nullptr);
  CHECK(elem->identifier.text == "Int32");
  auto const size =
    std::get_if<basedparse::Int_literal_expression>(&param_type->index->value);
  REQUIRE(size != nullptr);
  CHECK(size->literal.text == "4");
}

TEST_CASE("parse_expression - index: simple")
{
  auto fixture = Parse_fixture{"arr[0]"};
  auto const expr = fixture.parser.parse_expression();
  auto const idx = std::get_if<basedparse::Index_expression>(&expr->value);
  REQUIRE(idx != nullptr);
  auto const operand =
    std::get_if<basedparse::Identifier_expression>(&idx->operand->value);
  REQUIRE(operand != nullptr);
  CHECK(operand->identifier.text == "arr");
  CHECK(idx->lbracket.text == "[");
  auto const index =
    std::get_if<basedparse::Int_literal_expression>(&idx->index->value);
  REQUIRE(index != nullptr);
  CHECK(index->literal.text == "0");
  CHECK(idx->rbracket.text == "]");
}

TEST_CASE("parse_expression - index: chained")
{
  // arr[0][1] should parse as (arr[0])[1]
  auto fixture = Parse_fixture{"arr[0][1]"};
  auto const expr = fixture.parser.parse_expression();
  auto const outer = std::get_if<basedparse::Index_expression>(&expr->value);
  REQUIRE(outer != nullptr);
  auto const outer_index =
    std::get_if<basedparse::Int_literal_expression>(&outer->index->value);
  REQUIRE(outer_index != nullptr);
  CHECK(outer_index->literal.text == "1");
  auto const inner =
    std::get_if<basedparse::Index_expression>(&outer->operand->value);
  REQUIRE(inner != nullptr);
  auto const inner_operand =
    std::get_if<basedparse::Identifier_expression>(&inner->operand->value);
  REQUIRE(inner_operand != nullptr);
  CHECK(inner_operand->identifier.text == "arr");
  auto const inner_index =
    std::get_if<basedparse::Int_literal_expression>(&inner->index->value);
  REQUIRE(inner_index != nullptr);
  CHECK(inner_index->literal.text == "0");
}

TEST_CASE("parse_expression - index: expression index")
{
  auto fixture = Parse_fixture{"arr[i + 1]"};
  auto const expr = fixture.parser.parse_expression();
  auto const idx = std::get_if<basedparse::Index_expression>(&expr->value);
  REQUIRE(idx != nullptr);
  auto const index =
    std::get_if<basedparse::Binary_expression>(&idx->index->value);
  REQUIRE(index != nullptr);
  CHECK(index->op.text == "+");
}

TEST_CASE("parse_expression - index: binds tighter than binary op")
{
  // arr[0] + 1 should parse as (arr[0]) + 1
  auto fixture = Parse_fixture{"arr[0] + 1"};
  auto const expr = fixture.parser.parse_expression();
  auto const add = std::get_if<basedparse::Binary_expression>(&expr->value);
  REQUIRE(add != nullptr);
  CHECK(add->op.text == "+");
  CHECK(
    std::get_if<basedparse::Index_expression>(&add->left->value) != nullptr
  );
}

TEST_CASE("parse_expression - index: call then index")
{
  // f()[0] should parse as (f())[0]
  auto fixture = Parse_fixture{"f()[0]"};
  auto const expr = fixture.parser.parse_expression();
  auto const idx = std::get_if<basedparse::Index_expression>(&expr->value);
  REQUIRE(idx != nullptr);
  CHECK(
    std::get_if<basedparse::Call_expression>(&idx->operand->value) != nullptr
  );
}

TEST_CASE("parse_expression - index: in full program")
{
  CHECK(parses("let f = fn(buf: Int32[4]): Int32 => { return buf[0]; };"));
  CHECK(parses("let f = fn() => { arr[0]; };"));
  CHECK(parses("let f = fn() => { arr[i + 1]; };"));
  CHECK(parses("let f = fn() => { f()[0]; };"));
  CHECK(parses("let f = fn() => { arr[0][1]; };"));
}

TEST_CASE("parse_expression - index: rejects")
{
  CHECK_FALSE(parses("let f = fn() => { arr[0; };"));
}

TEST_CASE("parse_expression - dereference")
{
  auto fixture = Parse_fixture{"*p"};
  auto const expr = fixture.parser.parse_expression();
  auto const unary = std::get_if<basedparse::Unary_expression>(&expr->value);
  REQUIRE(unary != nullptr);
  CHECK(unary->op.text == "*");
  auto const operand =
    std::get_if<basedparse::Identifier_expression>(&unary->operand->value);
  REQUIRE(operand != nullptr);
  CHECK(operand->identifier.text == "p");
}

TEST_CASE("parse_expression - dereference: postfix binds tighter")
{
  // *p[0] should parse as *(p[0])
  auto fixture = Parse_fixture{"*p[0]"};
  auto const expr = fixture.parser.parse_expression();
  auto const unary = std::get_if<basedparse::Unary_expression>(&expr->value);
  REQUIRE(unary != nullptr);
  CHECK(unary->op.text == "*");
  CHECK(
    std::get_if<basedparse::Index_expression>(&unary->operand->value) != nullptr
  );
}

TEST_CASE("parse_expression - dereference: in binary expression")
{
  // *a + *b should parse as (*a) + (*b)
  auto fixture = Parse_fixture{"*a + *b"};
  auto const expr = fixture.parser.parse_expression();
  auto const add = std::get_if<basedparse::Binary_expression>(&expr->value);
  REQUIRE(add != nullptr);
  CHECK(add->op.text == "+");
  auto const left =
    std::get_if<basedparse::Unary_expression>(&add->left->value);
  REQUIRE(left != nullptr);
  CHECK(left->op.text == "*");
  auto const right =
    std::get_if<basedparse::Unary_expression>(&add->right->value);
  REQUIRE(right != nullptr);
  CHECK(right->op.text == "*");
}

TEST_CASE("parse_expression - dereference: in full program")
{
  CHECK(parses("let f = fn(p: *Int32): Int32 => { return *p; };"));
  CHECK(parses("let f = fn(p: *[]Int32): Int32 => { return *p[0]; };"));
}

TEST_CASE("parse_expression - address-of")
{
  auto fixture = Parse_fixture{"&x"};
  auto const expr = fixture.parser.parse_expression();
  auto const unary = std::get_if<basedparse::Unary_expression>(&expr->value);
  REQUIRE(unary != nullptr);
  CHECK(unary->op.text == "&");
  auto const operand =
    std::get_if<basedparse::Identifier_expression>(&unary->operand->value);
  REQUIRE(operand != nullptr);
  CHECK(operand->identifier.text == "x");
}

TEST_CASE("parse_expression - address-of: postfix binds tighter")
{
  // &a[0] should parse as &(a[0])
  auto fixture = Parse_fixture{"&a[0]"};
  auto const expr = fixture.parser.parse_expression();
  auto const unary = std::get_if<basedparse::Unary_expression>(&expr->value);
  REQUIRE(unary != nullptr);
  CHECK(unary->op.text == "&");
  CHECK(
    std::get_if<basedparse::Index_expression>(&unary->operand->value) != nullptr
  );
}

TEST_CASE("parse_expression - address-of: in binary expression")
{
  // &a + &b should parse as (&a) + (&b)
  auto fixture = Parse_fixture{"&a + &b"};
  auto const expr = fixture.parser.parse_expression();
  auto const add = std::get_if<basedparse::Binary_expression>(&expr->value);
  REQUIRE(add != nullptr);
  CHECK(add->op.text == "+");
  auto const left =
    std::get_if<basedparse::Unary_expression>(&add->left->value);
  REQUIRE(left != nullptr);
  CHECK(left->op.text == "&");
  auto const right =
    std::get_if<basedparse::Unary_expression>(&add->right->value);
  REQUIRE(right != nullptr);
  CHECK(right->op.text == "&");
}

TEST_CASE("parse_expression - address-of: in full program")
{
  CHECK(parses("let f = fn(x: Int32): &Int32 => { return &x; };"));
}

TEST_CASE("parse_expression - if: simple")
{
  auto fixture = Parse_fixture{"if x { 1 }"};
  auto const expr = fixture.parser.parse_expression();
  auto const if_expr = std::get_if<basedparse::If_expression>(&expr->value);
  REQUIRE(if_expr != nullptr);
  CHECK(if_expr->kw_if.text == "if");
  auto const cond =
    std::get_if<basedparse::Identifier_expression>(&if_expr->condition->value);
  REQUIRE(cond != nullptr);
  CHECK(cond->identifier.text == "x");
  REQUIRE(if_expr->then_block.tail != nullptr);
  CHECK(if_expr->else_if_parts.empty());
  CHECK_FALSE(if_expr->else_part.has_value());
}

TEST_CASE("parse_expression - if else")
{
  auto fixture = Parse_fixture{"if x { 1 } else { 0 }"};
  auto const expr = fixture.parser.parse_expression();
  auto const if_expr = std::get_if<basedparse::If_expression>(&expr->value);
  REQUIRE(if_expr != nullptr);
  CHECK(if_expr->else_if_parts.empty());
  REQUIRE(if_expr->else_part.has_value());
  CHECK(if_expr->else_part->kw_else.text == "else");
  REQUIRE(if_expr->else_part->body.tail != nullptr);
}

TEST_CASE("parse_expression - else if chain")
{
  auto fixture = Parse_fixture{"if a { 1 } else if b { 2 } else { 3 }"};
  auto const expr = fixture.parser.parse_expression();
  auto const if_expr = std::get_if<basedparse::If_expression>(&expr->value);
  REQUIRE(if_expr != nullptr);
  REQUIRE(if_expr->else_if_parts.size() == 1);
  CHECK(if_expr->else_if_parts[0].kw_else.text == "else");
  CHECK(if_expr->else_if_parts[0].kw_if.text == "if");
  auto const cond = std::get_if<basedparse::Identifier_expression>(
    &if_expr->else_if_parts[0].condition->value
  );
  REQUIRE(cond != nullptr);
  CHECK(cond->identifier.text == "b");
  REQUIRE(if_expr->else_if_parts[0].body.tail != nullptr);
  REQUIRE(if_expr->else_part.has_value());
  REQUIRE(if_expr->else_part->body.tail != nullptr);
}

TEST_CASE("parse_expression - if as expression statement")
{
  CHECK(parses("let f = fn() => { if x { 1; }; };"));
}

TEST_CASE("parse_expression - if else as initializer")
{
  CHECK(parses("let f = fn() => { let x = if cond { 1 } else { 0 }; };"));
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
    auto const expr = fixture.parser.parse_expression();
    auto const bin = std::get_if<basedparse::Binary_expression>(&expr->value);
    REQUIRE(bin != nullptr);
    CHECK(bin->op.text == op);
    auto const left =
      std::get_if<basedparse::Int_literal_expression>(&bin->left->value);
    REQUIRE(left != nullptr);
    CHECK(left->literal.text == "1");
    auto const right =
      std::get_if<basedparse::Int_literal_expression>(&bin->right->value);
    REQUIRE(right != nullptr);
    CHECK(right->literal.text == "2");
  }
}

TEST_CASE("parse_expression - additive before comparison")
{
  // 1 + 2 < 3 + 4 should parse as (1 + 2) < (3 + 4)
  auto fixture = Parse_fixture{"1 + 2 < 3 + 4"};
  auto const expr = fixture.parser.parse_expression();
  auto const lt = std::get_if<basedparse::Binary_expression>(&expr->value);
  REQUIRE(lt != nullptr);
  CHECK(lt->op.text == "<");
  auto const left =
    std::get_if<basedparse::Binary_expression>(&lt->left->value);
  REQUIRE(left != nullptr);
  CHECK(left->op.text == "+");
  auto const right =
    std::get_if<basedparse::Binary_expression>(&lt->right->value);
  REQUIRE(right != nullptr);
  CHECK(right->op.text == "+");
}

TEST_CASE("parse_expression - comparison before equality")
{
  // 1 < 2 == 3 > 4 should parse as (1 < 2) == (3 > 4)
  auto fixture = Parse_fixture{"1 < 2 == 3 > 4"};
  auto const expr = fixture.parser.parse_expression();
  auto const eq = std::get_if<basedparse::Binary_expression>(&expr->value);
  REQUIRE(eq != nullptr);
  CHECK(eq->op.text == "==");
  auto const left =
    std::get_if<basedparse::Binary_expression>(&eq->left->value);
  REQUIRE(left != nullptr);
  CHECK(left->op.text == "<");
  auto const right =
    std::get_if<basedparse::Binary_expression>(&eq->right->value);
  REQUIRE(right != nullptr);
  CHECK(right->op.text == ">");
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
  auto const expr = fixture.parser.parse_expression();
  auto const bin = std::get_if<basedparse::Binary_expression>(&expr->value);
  REQUIRE(bin != nullptr);
  CHECK(bin->op.text == "=");
  auto const left =
    std::get_if<basedparse::Identifier_expression>(&bin->left->value);
  REQUIRE(left != nullptr);
  CHECK(left->identifier.text == "x");
  auto const right =
    std::get_if<basedparse::Int_literal_expression>(&bin->right->value);
  REQUIRE(right != nullptr);
  CHECK(right->literal.text == "1");
}

TEST_CASE("parse_expression - assign: right-associative")
{
  // a = b = 1 should parse as a = (b = 1)
  auto fixture = Parse_fixture{"a = b = 1"};
  auto const expr = fixture.parser.parse_expression();
  auto const outer = std::get_if<basedparse::Binary_expression>(&expr->value);
  REQUIRE(outer != nullptr);
  CHECK(outer->op.text == "=");
  auto const left =
    std::get_if<basedparse::Identifier_expression>(&outer->left->value);
  REQUIRE(left != nullptr);
  CHECK(left->identifier.text == "a");
  auto const inner =
    std::get_if<basedparse::Binary_expression>(&outer->right->value);
  REQUIRE(inner != nullptr);
  CHECK(inner->op.text == "=");
  auto const inner_left =
    std::get_if<basedparse::Identifier_expression>(&inner->left->value);
  REQUIRE(inner_left != nullptr);
  CHECK(inner_left->identifier.text == "b");
}

TEST_CASE("parse_expression - assign: lower precedence than equality")
{
  // x = a == b should parse as x = (a == b)
  auto fixture = Parse_fixture{"x = a == b"};
  auto const expr = fixture.parser.parse_expression();
  auto const assign = std::get_if<basedparse::Binary_expression>(&expr->value);
  REQUIRE(assign != nullptr);
  CHECK(assign->op.text == "=");
  auto const rhs =
    std::get_if<basedparse::Binary_expression>(&assign->right->value);
  REQUIRE(rhs != nullptr);
  CHECK(rhs->op.text == "==");
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
  auto const stmt = fixture.parser.parse_statement();
  auto const while_stmt = std::get_if<basedparse::While_statement>(&stmt.value);
  REQUIRE(while_stmt != nullptr);
  CHECK(while_stmt->kw_while.text == "while");
  auto const cond = std::get_if<basedparse::Identifier_expression>(
    &while_stmt->condition->value
  );
  REQUIRE(cond != nullptr);
  CHECK(cond->identifier.text == "x");
  CHECK(while_stmt->body.statements.empty());
}

TEST_CASE("parse_statement - while: with body")
{
  auto fixture = Parse_fixture{"while n > 0 { f(n); }"};
  auto const stmt = fixture.parser.parse_statement();
  auto const while_stmt = std::get_if<basedparse::While_statement>(&stmt.value);
  REQUIRE(while_stmt != nullptr);
  auto const cond =
    std::get_if<basedparse::Binary_expression>(&while_stmt->condition->value);
  REQUIRE(cond != nullptr);
  CHECK(cond->op.text == ">");
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
  auto const stmt = fixture.parser.parse_statement();
  auto const let_stmt = std::get_if<basedparse::Let_statement>(&stmt.value);
  REQUIRE(let_stmt != nullptr);
  CHECK(let_stmt->kw_mut.has_value());
  CHECK(let_stmt->kw_mut->text == "mut");
  CHECK(let_stmt->name.text == "x");
}

TEST_CASE("Parser - quicksort.based parses successfully")
{
  auto file = std::ifstream{std::string{EXAMPLES_PATH} + "/quicksort.based"};
  auto binary_stream = basedlex::Istream_binary_stream{&file};
  auto char_stream = basedlex::Utf8_char_stream{&binary_stream};
  auto lexeme_stream = basedlex::Lexeme_stream{&char_stream};
  auto reader = basedlex::Lexeme_stream_reader{&lexeme_stream};
  auto parser = basedparse::Parser{&reader};
  auto const unit = parser.parse_translation_unit();
  REQUIRE(unit != nullptr);
  CHECK(unit->function_definitions.size() == 3);
}

TEST_CASE("parse_expression - prefix bracket unsized array type")
{
  auto fixture = Parse_fixture{"[]Int32"};
  auto const expr = fixture.parser.parse_expression();
  auto const prefix =
    std::get_if<basedparse::Prefix_bracket_expression>(&expr->value);
  REQUIRE(prefix != nullptr);
  CHECK(prefix->lbracket.text == "[");
  CHECK(prefix->size == nullptr);
  CHECK(prefix->rbracket.text == "]");
  auto const operand =
    std::get_if<basedparse::Identifier_expression>(&prefix->operand->value);
  REQUIRE(operand != nullptr);
  CHECK(operand->identifier.text == "Int32");
}

TEST_CASE("parse_expression - prefix bracket sized array type")
{
  auto fixture = Parse_fixture{"[4]Int32"};
  auto const expr = fixture.parser.parse_expression();
  auto const prefix =
    std::get_if<basedparse::Prefix_bracket_expression>(&expr->value);
  REQUIRE(prefix != nullptr);
  CHECK(prefix->lbracket.text == "[");
  REQUIRE(prefix->size != nullptr);
  auto const size =
    std::get_if<basedparse::Int_literal_expression>(&prefix->size->value);
  REQUIRE(size != nullptr);
  CHECK(size->literal.text == "4");
  CHECK(prefix->rbracket.text == "]");
  auto const operand =
    std::get_if<basedparse::Identifier_expression>(&prefix->operand->value);
  REQUIRE(operand != nullptr);
  CHECK(operand->identifier.text == "Int32");
}

TEST_CASE("parse_expression - pointer to unsized array type")
{
  auto fixture = Parse_fixture{"*[]Int32"};
  auto const expr = fixture.parser.parse_expression();
  auto const unary = std::get_if<basedparse::Unary_expression>(&expr->value);
  REQUIRE(unary != nullptr);
  CHECK(unary->op.text == "*");
  auto const prefix =
    std::get_if<basedparse::Prefix_bracket_expression>(&unary->operand->value);
  REQUIRE(prefix != nullptr);
  CHECK(prefix->size == nullptr);
  auto const operand =
    std::get_if<basedparse::Identifier_expression>(&prefix->operand->value);
  REQUIRE(operand != nullptr);
  CHECK(operand->identifier.text == "Int32");
}

TEST_CASE("parse_expression - *mut prefix")
{
  auto fixture = Parse_fixture{"*mut Int32"};
  auto const expr = fixture.parser.parse_expression();
  auto const unary = std::get_if<basedparse::Unary_expression>(&expr->value);
  REQUIRE(unary != nullptr);
  CHECK(unary->op.text == "*mut");
  CHECK(unary->op.token == basedlex::Token::star_mut);
  auto const operand =
    std::get_if<basedparse::Identifier_expression>(&unary->operand->value);
  REQUIRE(operand != nullptr);
  CHECK(operand->identifier.text == "Int32");
}

TEST_CASE("parse_expression - *mut []Int32 as mutable pointer to unsized array")
{
  auto fixture = Parse_fixture{"*mut []Int32"};
  auto const expr = fixture.parser.parse_expression();
  auto const unary = std::get_if<basedparse::Unary_expression>(&expr->value);
  REQUIRE(unary != nullptr);
  CHECK(unary->op.token == basedlex::Token::star_mut);
  auto const prefix =
    std::get_if<basedparse::Prefix_bracket_expression>(&unary->operand->value);
  REQUIRE(prefix != nullptr);
  CHECK(prefix->size == nullptr);
  auto const operand =
    std::get_if<basedparse::Identifier_expression>(&prefix->operand->value);
  REQUIRE(operand != nullptr);
  CHECK(operand->identifier.text == "Int32");
}

TEST_CASE("Parser - accepts prefix type syntax")
{
  CHECK(parses("let f = fn(x: []Int32): Void => { };"));
  CHECK(parses("let f = fn(x: [4]Int32): Void => { };"));
  CHECK(parses("let f = fn(x: *Int32): Void => { };"));
  CHECK(parses("let f = fn(x: *mut Int32): Void => { };"));
  CHECK(parses("let f = fn(x: *mut []Int32): Void => { };"));
  CHECK(parses("let f = fn(x: *[]Int32): Void => { };"));
  CHECK(parses("let f = fn(x: [][4]Int32): Void => { };"));
}
