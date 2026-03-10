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
  REQUIRE(unit->statements.size() == 1);
  auto const fn_def =
    std::get_if<basedparse::Function_definition>(&unit->statements[0].value);
  REQUIRE(fn_def != nullptr);
  CHECK(fn_def->kw_let.text == "let");
  CHECK(fn_def->name.text == "main");
  CHECK(fn_def->eq.text == "=");
  CHECK(fn_def->function.kw_fn.text == "fn");
  CHECK(fn_def->function.lparen.text == "(");
  CHECK(fn_def->function.rparen.text == ")");
  REQUIRE(fn_def->function.return_type_specifier.has_value());
  auto const return_type = std::get_if<basedparse::Identifier_type_expression>(
    &fn_def->function.return_type_specifier->type_expression.value
  );
  REQUIRE(return_type != nullptr);
  CHECK(return_type->identifier.text == "i32");
  REQUIRE(fn_def->function.body != nullptr);
  CHECK(fn_def->function.body->lbrace.text == "{");
  REQUIRE(fn_def->function.body->statements.size() == 1);
  auto const ret_stmt = std::get_if<basedparse::Return_statement>(
    &fn_def->function.body->statements[0].value
  );
  REQUIRE(ret_stmt != nullptr);
  CHECK(ret_stmt->kw_return.text == "return");
  auto const int_lit =
    std::get_if<basedparse::Int_literal_expression>(&ret_stmt->value.value);
  REQUIRE(int_lit != nullptr);
  CHECK(int_lit->literal.text == "0");
  CHECK(ret_stmt->semicolon.text == ";");
  CHECK(fn_def->function.body->rbrace.text == "}");
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
  REQUIRE(unit->statements.size() == 3);
  auto const id_def =
    std::get_if<basedparse::Function_definition>(&unit->statements[0].value);
  auto const first_def =
    std::get_if<basedparse::Function_definition>(&unit->statements[1].value);
  auto const main_def =
    std::get_if<basedparse::Function_definition>(&unit->statements[2].value);
  REQUIRE(id_def != nullptr);
  REQUIRE(first_def != nullptr);
  REQUIRE(main_def != nullptr);
  CHECK(id_def->name.text == "id");
  CHECK(first_def->name.text == "first");
  CHECK(main_def->name.text == "main");
  // id: fn(x: i32) -> i32 { return x; }
  REQUIRE(id_def->function.parameters.size() == 1);
  CHECK(id_def->function.parameters[0].name.text == "x");
  auto const id_param_type =
    std::get_if<basedparse::Identifier_type_expression>(
      &id_def->function.parameters[0].type_expression.value
    );
  REQUIRE(id_param_type != nullptr);
  CHECK(id_param_type->identifier.text == "i32");
  REQUIRE(id_def->function.return_type_specifier.has_value());
  auto const id_ret_type = std::get_if<basedparse::Identifier_type_expression>(
    &id_def->function.return_type_specifier->type_expression.value
  );
  REQUIRE(id_ret_type != nullptr);
  CHECK(id_ret_type->identifier.text == "i32");
  REQUIRE(id_def->function.body->statements.size() == 1);
  auto const id_ret = std::get_if<basedparse::Return_statement>(
    &id_def->function.body->statements[0].value
  );
  REQUIRE(id_ret != nullptr);
  auto const id_ret_val =
    std::get_if<basedparse::Identifier_expression>(&id_ret->value.value);
  REQUIRE(id_ret_val != nullptr);
  CHECK(id_ret_val->identifier.text == "x");
  // first: fn(x: i32, y: i32) -> i32 { return x; }
  REQUIRE(first_def->function.parameters.size() == 2);
  CHECK(first_def->function.parameters[0].name.text == "x");
  auto const first_param0_type =
    std::get_if<basedparse::Identifier_type_expression>(
      &first_def->function.parameters[0].type_expression.value
    );
  REQUIRE(first_param0_type != nullptr);
  CHECK(first_param0_type->identifier.text == "i32");
  CHECK(first_def->function.parameters[1].name.text == "y");
  auto const first_param1_type =
    std::get_if<basedparse::Identifier_type_expression>(
      &first_def->function.parameters[1].type_expression.value
    );
  REQUIRE(first_param1_type != nullptr);
  CHECK(first_param1_type->identifier.text == "i32");
  REQUIRE(first_def->function.return_type_specifier.has_value());
  auto const first_ret_type =
    std::get_if<basedparse::Identifier_type_expression>(
      &first_def->function.return_type_specifier->type_expression.value
    );
  REQUIRE(first_ret_type != nullptr);
  CHECK(first_ret_type->identifier.text == "i32");
  REQUIRE(first_def->function.body->statements.size() == 1);
  auto const first_ret = std::get_if<basedparse::Return_statement>(
    &first_def->function.body->statements[0].value
  );
  REQUIRE(first_ret != nullptr);
  auto const first_ret_val =
    std::get_if<basedparse::Identifier_expression>(&first_ret->value.value);
  REQUIRE(first_ret_val != nullptr);
  CHECK(first_ret_val->identifier.text == "x");
  // main: fn() -> i32 { return first(id(42), 0); }
  REQUIRE(main_def->function.return_type_specifier.has_value());
  auto const main_ret_type =
    std::get_if<basedparse::Identifier_type_expression>(
      &main_def->function.return_type_specifier->type_expression.value
    );
  REQUIRE(main_ret_type != nullptr);
  CHECK(main_ret_type->identifier.text == "i32");
  REQUIRE(main_def->function.body->statements.size() == 1);
  auto const main_ret = std::get_if<basedparse::Return_statement>(
    &main_def->function.body->statements[0].value
  );
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
  REQUIRE(unit->statements.size() == 2);
  auto const foo =
    std::get_if<basedparse::Function_definition>(&unit->statements[0].value);
  auto const main =
    std::get_if<basedparse::Function_definition>(&unit->statements[1].value);
  REQUIRE(foo != nullptr);
  REQUIRE(main != nullptr);
  CHECK(foo->name.text == "foo");
  CHECK(main->name.text == "main");
  // foo returns fn() -> i32 { return 0; }() — a call with an fn expression
  // callee
  REQUIRE(foo->function.body->statements.size() == 1);
  auto const foo_ret = std::get_if<basedparse::Return_statement>(
    &foo->function.body->statements[0].value
  );
  REQUIRE(foo_ret != nullptr);
  auto const foo_call =
    std::get_if<basedparse::Call_expression>(&foo_ret->value.value);
  REQUIRE(foo_call != nullptr);
  CHECK(foo_call->lparen.text == "(");
  CHECK(foo_call->rparen.text == ")");
  CHECK(
    std::get_if<basedparse::Fn_expression>(&foo_call->callee->value) != nullptr
  );
  // main returns foo() — a call with an identifier callee
  REQUIRE(main->function.body->statements.size() == 1);
  auto const main_ret = std::get_if<basedparse::Return_statement>(
    &main->function.body->statements[0].value
  );
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
  CHECK(parses("let main = fn() -> i32 { return 0; };"));
  CHECK(parses("let main = fn() -> i32 { };"));
  CHECK(parses(
    "let main = fn() -> i32 { return 0; };\n"
    "let other = fn() -> i32 { return 1; };"
  ));
  CHECK(parses("let main = fn() -> void { x; };"));
  CHECK(parses("let main = fn() -> void { let x = 42; };"));
  CHECK(parses("let main = fn() -> void { (x); };"));
  CHECK(parses("let main = fn() -> void { ((42)); };"));
  CHECK(parses("let f = fn(x: i32) -> i32 { return x; };"));
  CHECK(parses("let f = fn(x: i32, y: i32) -> i32 { return x; };"));
  CHECK(parses("let f = fn(x: i32,) -> i32 { return x; };"));
  CHECK(parses("let f = fn(mut x: i32) -> void { };"));
  CHECK(parses("let f = fn(x: i32, mut y: i32) -> void { };"));
  CHECK(parses("let main = fn() -> void { f(1); };"));
  CHECK(parses("let main = fn() -> void { f(1, 2); };"));
  CHECK(parses("let main = fn() -> void { f(1,); };"));
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
  CHECK_FALSE(parses("let x = fn() -> i32 { }"));
  CHECK_FALSE(parses("return 0;"));
  CHECK_FALSE(parses("{"));
  CHECK_FALSE(parses("42"));
  CHECK_FALSE(parses("let = fn() -> i32 { };"));
  CHECK_FALSE(parses("let x = fn() -> i32 { return; };"));
  CHECK_FALSE(parses("let x = fn() -> i32 { return (; };"));
  CHECK_FALSE(parses("let x = fn() -> i32 { return (); };"));
  // malformed parameter declarations
  CHECK_FALSE(parses("let f = fn(x) -> i32 { };"));
  CHECK_FALSE(parses("let f = fn(x:) -> i32 { };"));
  CHECK_FALSE(parses("let f = fn(x: i32 y: i32) -> i32 { };"));
  // malformed argument lists
  CHECK_FALSE(parses("let main = fn() -> void { f(,); };"));
  CHECK_FALSE(parses("let main = fn() -> void { f(1 2); };"));
  // malformed expressions: missing operands
  CHECK_FALSE(parses("let x = fn() -> i32 { return 1 +; };"));
  CHECK_FALSE(parses("let x = fn() -> i32 { return 1 *; };"));
  CHECK(parses("let x = fn() -> i32 { return * 1; };"));
  CHECK(parses("let x = fn() -> i32 { return 1 + * 2; };"));
  CHECK_FALSE(parses("let x = fn() -> i32 { return +; };"));
  CHECK_FALSE(parses("let x = fn() -> i32 { return -; };"));
  // malformed array types
  CHECK_FALSE(parses("let f = fn(x: [4]) -> void { };"));
  CHECK_FALSE(parses("let f = fn(x: i32[4) -> void { };"));
  CHECK_FALSE(parses("let f = fn(x: [4]) -> void { };"));
}

TEST_CASE("parse_translation_unit - empty")
{
  auto fixture = Parse_fixture{""};
  auto const unit = fixture.parser.parse_translation_unit();
  CHECK(unit->statements.empty());
}

TEST_CASE("parse_translation_unit - multiple functions")
{
  auto fixture = Parse_fixture{"let a = fn() -> i32 { return 1; };\n"
                               "let b = fn() -> i32 { return 2; };"};
  auto const unit = fixture.parser.parse_translation_unit();
  REQUIRE(unit->statements.size() == 2);
  auto const a =
    std::get_if<basedparse::Function_definition>(&unit->statements[0].value);
  auto const b =
    std::get_if<basedparse::Function_definition>(&unit->statements[1].value);
  REQUIRE(a != nullptr);
  REQUIRE(b != nullptr);
  CHECK(a->name.text == "a");
  CHECK(b->name.text == "b");
}

TEST_CASE("parse_function_definition")
{
  auto fixture = Parse_fixture{"let main = fn() -> i32 { return 0; };"};
  auto const fn_def = fixture.parser.parse_function_definition();
  CHECK(fn_def.kw_let.text == "let");
  CHECK(fn_def.name.text == "main");
  CHECK(fn_def.eq.text == "=");
  CHECK(fn_def.function.kw_fn.text == "fn");
  REQUIRE(fn_def.function.body != nullptr);
  CHECK(fn_def.function.body->statements.size() == 1);
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
  CHECK(parses("let f = fn() { { 42 }; };"));
}

TEST_CASE("parse_expression - block as initializer")
{
  CHECK(parses("let f = fn() { let x = { let a = 1; a + 1 }; };"));
}

TEST_CASE("parse_block_expression - fn body parses tail syntactically")
{
  auto fixture = Parse_fixture{"fn() -> i32 { 42 }"};
  auto const fn = fixture.parser.parse_fn_expression();
  REQUIRE(fn.body != nullptr);
  CHECK(fn.body->statements.empty());
  REQUIRE(fn.body->tail != nullptr);
  auto const lit =
    std::get_if<basedparse::Int_literal_expression>(&fn.body->tail->value);
  REQUIRE(lit != nullptr);
  CHECK(lit->literal.text == "42");
}

TEST_CASE("parse_fn_expression")
{
  auto fixture = Parse_fixture{"fn() -> i32 { return 0; }"};
  auto const fn = fixture.parser.parse_fn_expression();
  CHECK(fn.kw_fn.text == "fn");
  CHECK(fn.lparen.text == "(");
  CHECK(fn.rparen.text == ")");
  REQUIRE(fn.return_type_specifier.has_value());
  auto const ret_type = std::get_if<basedparse::Identifier_type_expression>(
    &fn.return_type_specifier->type_expression.value
  );
  REQUIRE(ret_type != nullptr);
  CHECK(ret_type->identifier.text == "i32");
  REQUIRE(fn.body != nullptr);
  CHECK(fn.body->statements.size() == 1);
}

TEST_CASE("parse_fn_expression - mut parameter")
{
  auto fixture = Parse_fixture{"fn(mut x: i32) { }"};
  auto const fn = fixture.parser.parse_fn_expression();
  REQUIRE(fn.parameters.size() == 1);
  REQUIRE(fn.parameters[0].kw_mut.has_value());
  CHECK(fn.parameters[0].kw_mut->text == "mut");
  CHECK(fn.parameters[0].name.text == "x");
}

TEST_CASE("parse_fn_expression - mixed mut and non-mut parameters")
{
  auto fixture = Parse_fixture{"fn(x: i32, mut y: i32) { }"};
  auto const fn = fixture.parser.parse_fn_expression();
  REQUIRE(fn.parameters.size() == 2);
  CHECK_FALSE(fn.parameters[0].kw_mut.has_value());
  CHECK(fn.parameters[0].name.text == "x");
  REQUIRE(fn.parameters[1].kw_mut.has_value());
  CHECK(fn.parameters[1].name.text == "y");
}

TEST_CASE("parse_fn_expression - no return type")
{
  auto fixture = Parse_fixture{"fn() { }"};
  auto const fn = fixture.parser.parse_fn_expression();
  CHECK_FALSE(fn.return_type_specifier.has_value());
  REQUIRE(fn.body != nullptr);
  CHECK(fn.body->statements.empty());
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

TEST_CASE("parse_type_expression - identifier")
{
  auto fixture = Parse_fixture{"i32"};
  auto const type = fixture.parser.parse_type_expression();
  auto const expr =
    std::get_if<basedparse::Identifier_type_expression>(&type->value);
  REQUIRE(expr != nullptr);
  CHECK(expr->identifier.text == "i32");
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
  auto fixture = Parse_fixture{"fn() -> i32 { }"};
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

TEST_CASE("parse_type_expression - dispatches to identifier type")
{
  auto fixture = Parse_fixture{"void"};
  auto const expr = fixture.parser.parse_type_expression();
  CHECK(
    std::get_if<basedparse::Identifier_type_expression>(&expr->value) != nullptr
  );
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

TEST_CASE("parse_type_expression - array")
{
  auto fixture = Parse_fixture{"i32[4]"};
  auto const type = fixture.parser.parse_type_expression();
  auto const array =
    std::get_if<basedparse::Array_type_expression>(&type->value);
  REQUIRE(array != nullptr);
  auto const elem = std::get_if<basedparse::Identifier_type_expression>(
    &array->element_type->value
  );
  REQUIRE(elem != nullptr);
  CHECK(elem->identifier.text == "i32");
  auto const size =
    std::get_if<basedparse::Int_literal_expression>(&array->size->value);
  REQUIRE(size != nullptr);
  CHECK(size->literal.text == "4");
}

TEST_CASE("parse_type_expression - array with expression size")
{
  auto fixture = Parse_fixture{"i32[n]"};
  auto const type = fixture.parser.parse_type_expression();
  auto const array =
    std::get_if<basedparse::Array_type_expression>(&type->value);
  REQUIRE(array != nullptr);
  auto const size =
    std::get_if<basedparse::Identifier_expression>(&array->size->value);
  REQUIRE(size != nullptr);
  CHECK(size->identifier.text == "n");
}

TEST_CASE("parse_type_expression - nested array")
{
  // i32[4][2] parses as (i32[4])[2]
  auto fixture = Parse_fixture{"i32[4][2]"};
  auto const type = fixture.parser.parse_type_expression();
  auto const outer =
    std::get_if<basedparse::Array_type_expression>(&type->value);
  REQUIRE(outer != nullptr);
  auto const outer_size =
    std::get_if<basedparse::Int_literal_expression>(&outer->size->value);
  REQUIRE(outer_size != nullptr);
  CHECK(outer_size->literal.text == "2");
  auto const inner =
    std::get_if<basedparse::Array_type_expression>(&outer->element_type->value);
  REQUIRE(inner != nullptr);
  auto const inner_size =
    std::get_if<basedparse::Int_literal_expression>(&inner->size->value);
  REQUIRE(inner_size != nullptr);
  CHECK(inner_size->literal.text == "4");
  auto const elem = std::get_if<basedparse::Identifier_type_expression>(
    &inner->element_type->value
  );
  REQUIRE(elem != nullptr);
  CHECK(elem->identifier.text == "i32");
}

TEST_CASE("parse_fn_expression - array parameter")
{
  auto fixture = Parse_fixture{"fn(buf: i32[4]) { }"};
  auto const fn = fixture.parser.parse_fn_expression();
  REQUIRE(fn.parameters.size() == 1);
  CHECK(fn.parameters[0].name.text == "buf");
  auto const param_type = std::get_if<basedparse::Array_type_expression>(
    &fn.parameters[0].type_expression.value
  );
  REQUIRE(param_type != nullptr);
  auto const elem = std::get_if<basedparse::Identifier_type_expression>(
    &param_type->element_type->value
  );
  REQUIRE(elem != nullptr);
  CHECK(elem->identifier.text == "i32");
  auto const size =
    std::get_if<basedparse::Int_literal_expression>(&param_type->size->value);
  REQUIRE(size != nullptr);
  CHECK(size->literal.text == "4");
}

TEST_CASE("parse_expression - constructor: empty")
{
  auto fixture = Parse_fixture{"new Foo{}"};
  auto const expr = fixture.parser.parse_expression();
  auto const ctor =
    std::get_if<basedparse::Constructor_expression>(&expr->value);
  REQUIRE(ctor != nullptr);
  CHECK(ctor->kw_new.text == "new");
  auto const type =
    std::get_if<basedparse::Identifier_type_expression>(&ctor->type.value);
  REQUIRE(type != nullptr);
  CHECK(type->identifier.text == "Foo");
  CHECK(ctor->arguments.empty());
}

TEST_CASE("parse_expression - constructor: multiple arguments")
{
  auto fixture = Parse_fixture{"new Foo{1, x, 2 + 3}"};
  auto const expr = fixture.parser.parse_expression();
  auto const ctor =
    std::get_if<basedparse::Constructor_expression>(&expr->value);
  REQUIRE(ctor != nullptr);
  REQUIRE(ctor->arguments.size() == 3);
  auto const first =
    std::get_if<basedparse::Int_literal_expression>(&ctor->arguments[0].value);
  REQUIRE(first != nullptr);
  CHECK(first->literal.text == "1");
  auto const second =
    std::get_if<basedparse::Identifier_expression>(&ctor->arguments[1].value);
  REQUIRE(second != nullptr);
  CHECK(second->identifier.text == "x");
  CHECK(
    std::get_if<basedparse::Binary_expression>(&ctor->arguments[2].value) !=
    nullptr
  );
}

TEST_CASE("parse_expression - constructor: array type")
{
  auto fixture = Parse_fixture{"new i32[4] { 1, 2, 3, 4 }"};
  auto const expr = fixture.parser.parse_expression();
  auto const ctor =
    std::get_if<basedparse::Constructor_expression>(&expr->value);
  REQUIRE(ctor != nullptr);
  auto const type =
    std::get_if<basedparse::Array_type_expression>(&ctor->type.value);
  REQUIRE(type != nullptr);
  auto const elem = std::get_if<basedparse::Identifier_type_expression>(
    &type->element_type->value
  );
  REQUIRE(elem != nullptr);
  CHECK(elem->identifier.text == "i32");
  REQUIRE(ctor->arguments.size() == 4);
}

TEST_CASE("parse_expression - constructor: in full program")
{
  CHECK(parses("let f = fn() { let x = new Foo{1, 2}; };"));
  CHECK(parses("let f = fn() { let x = new i32[3] { 1, 2, 3 }; };"));
}

TEST_CASE("parse_expression - constructor: trailing comma")
{
  auto fixture = Parse_fixture{"new Foo{1, 2,}"};
  auto const expr = fixture.parser.parse_expression();
  auto const ctor =
    std::get_if<basedparse::Constructor_expression>(&expr->value);
  REQUIRE(ctor != nullptr);
  REQUIRE(ctor->arguments.size() == 2);
  CHECK(ctor->argument_commas.size() == 2);
}

TEST_CASE("parse_expression - constructor: rejects")
{
  CHECK_FALSE(parses("let x = fn() { new Foo{;}; };"));
  CHECK_FALSE(parses("let x = fn() { new Foo{1 2}; };"));
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
  CHECK(parses("let f = fn(buf: i32[4]) -> i32 { return buf[0]; };"));
  CHECK(parses("let f = fn() { arr[0]; };"));
  CHECK(parses("let f = fn() { arr[i + 1]; };"));
  CHECK(parses("let f = fn() { f()[0]; };"));
  CHECK(parses("let f = fn() { arr[0][1]; };"));
}

TEST_CASE("parse_expression - index: rejects")
{
  CHECK_FALSE(parses("let f = fn() { arr[]; };"));
  CHECK_FALSE(parses("let f = fn() { arr[0; };"));
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
  CHECK(parses("let f = fn(p: i32*) -> i32 { return *p; };"));
  CHECK(parses("let f = fn(p: i32[]*) -> i32 { return *p[0]; };"));
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
  CHECK(parses("let f = fn(x: i32) -> i32* { return &x; };"));
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
  CHECK_FALSE(if_expr->else_clause.has_value());
}

TEST_CASE("parse_expression - if else")
{
  auto fixture = Parse_fixture{"if x { 1 } else { 0 }"};
  auto const expr = fixture.parser.parse_expression();
  auto const if_expr = std::get_if<basedparse::If_expression>(&expr->value);
  REQUIRE(if_expr != nullptr);
  REQUIRE(if_expr->else_clause.has_value());
  CHECK(if_expr->else_clause->kw_else.text == "else");
  auto const else_block = std::get_if<basedparse::Block_expression>(
    &if_expr->else_clause->body->value
  );
  REQUIRE(else_block != nullptr);
  REQUIRE(else_block->tail != nullptr);
}

TEST_CASE("parse_expression - else if chain")
{
  auto fixture = Parse_fixture{"if a { 1 } else if b { 2 } else { 3 }"};
  auto const expr = fixture.parser.parse_expression();
  auto const if1 = std::get_if<basedparse::If_expression>(&expr->value);
  REQUIRE(if1 != nullptr);
  REQUIRE(if1->else_clause.has_value());
  auto const if2 =
    std::get_if<basedparse::If_expression>(&if1->else_clause->body->value);
  REQUIRE(if2 != nullptr);
  REQUIRE(if2->else_clause.has_value());
  auto const else_block =
    std::get_if<basedparse::Block_expression>(&if2->else_clause->body->value);
  REQUIRE(else_block != nullptr);
}

TEST_CASE("parse_expression - if as expression statement")
{
  CHECK(parses("let f = fn() { if x { 1; }; };"));
}

TEST_CASE("parse_expression - if else as initializer")
{
  CHECK(parses("let f = fn() { let x = if cond { 1 } else { 0 }; };"));
}

TEST_CASE("parse_expression - if in full program")
{
  CHECK(parses(
    "let f = fn(x: i32) -> i32 {"
    "  return if x { 1 } else { 0 };"
    "};"
  ));
  CHECK(parses(
    "let f = fn(x: i32) -> i32 {"
    "  if x { return 1; };"
    "  return 0;"
    "};"
  ));
  CHECK(parses(
    "let f = fn(x: i32) -> i32 {"
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
  CHECK(parses("let f = fn(x: i32) -> i32 { return if x < 10 { 0 } else { 1 }; };"));
  CHECK(parses("let f = fn(a: i32, b: i32) -> i32 { return a == b; };"));
  CHECK(parses("let f = fn(a: i32, b: i32) -> i32 { return a != b; };"));
}

TEST_CASE("parse_type_expression - pointer")
{
  auto fixture = Parse_fixture{"i32*"};
  auto const type = fixture.parser.parse_type_expression();
  auto const ptr =
    std::get_if<basedparse::Pointer_type_expression>(&type->value);
  REQUIRE(ptr != nullptr);
  CHECK_FALSE(ptr->kw_mut.has_value());
  CHECK(ptr->star.text == "*");
  auto const pointee = std::get_if<basedparse::Identifier_type_expression>(
    &ptr->pointee_type->value
  );
  REQUIRE(pointee != nullptr);
  CHECK(pointee->identifier.text == "i32");
}

TEST_CASE("parse_type_expression - mutable pointer")
{
  auto fixture = Parse_fixture{"i32 mut*"};
  auto const type = fixture.parser.parse_type_expression();
  auto const ptr =
    std::get_if<basedparse::Pointer_type_expression>(&type->value);
  REQUIRE(ptr != nullptr);
  REQUIRE(ptr->kw_mut.has_value());
  CHECK(ptr->kw_mut->text == "mut");
  CHECK(ptr->star.text == "*");
  auto const pointee = std::get_if<basedparse::Identifier_type_expression>(
    &ptr->pointee_type->value
  );
  REQUIRE(pointee != nullptr);
  CHECK(pointee->identifier.text == "i32");
}

TEST_CASE("parse_type_expression - unsized array")
{
  auto fixture = Parse_fixture{"i32[]"};
  auto const type = fixture.parser.parse_type_expression();
  auto const array =
    std::get_if<basedparse::Array_type_expression>(&type->value);
  REQUIRE(array != nullptr);
  CHECK(array->size == nullptr);
  auto const elem = std::get_if<basedparse::Identifier_type_expression>(
    &array->element_type->value
  );
  REQUIRE(elem != nullptr);
  CHECK(elem->identifier.text == "i32");
}

TEST_CASE("parse_type_expression - unsized array pointer")
{
  // i32[]* — pointer to unsized array of i32
  auto fixture = Parse_fixture{"i32[]*"};
  auto const type = fixture.parser.parse_type_expression();
  auto const ptr =
    std::get_if<basedparse::Pointer_type_expression>(&type->value);
  REQUIRE(ptr != nullptr);
  CHECK_FALSE(ptr->kw_mut.has_value());
  auto const array =
    std::get_if<basedparse::Array_type_expression>(&ptr->pointee_type->value);
  REQUIRE(array != nullptr);
  CHECK(array->size == nullptr);
}

TEST_CASE("parse_type_expression - unsized array mutable pointer")
{
  // i32[] mut* — pointer to mutable unsized array
  auto fixture = Parse_fixture{"i32[] mut*"};
  auto const type = fixture.parser.parse_type_expression();
  auto const ptr =
    std::get_if<basedparse::Pointer_type_expression>(&type->value);
  REQUIRE(ptr != nullptr);
  REQUIRE(ptr->kw_mut.has_value());
  auto const array =
    std::get_if<basedparse::Array_type_expression>(&ptr->pointee_type->value);
  REQUIRE(array != nullptr);
  CHECK(array->size == nullptr);
}

TEST_CASE("parse_type_expression - nested pointer array")
{
  // i32[4] mut*[8]* — pointer to (array of 8 of (pointer to mutable (array of 4
  // of i32)))
  auto fixture = Parse_fixture{"i32[4] mut*[8]*"};
  auto const type = fixture.parser.parse_type_expression();
  // outermost: *
  auto const outer_ptr =
    std::get_if<basedparse::Pointer_type_expression>(&type->value);
  REQUIRE(outer_ptr != nullptr);
  CHECK_FALSE(outer_ptr->kw_mut.has_value());
  // [8]
  auto const array8 = std::get_if<basedparse::Array_type_expression>(
    &outer_ptr->pointee_type->value
  );
  REQUIRE(array8 != nullptr);
  auto const size8 =
    std::get_if<basedparse::Int_literal_expression>(&array8->size->value);
  REQUIRE(size8 != nullptr);
  CHECK(size8->literal.text == "8");
  // mut*
  auto const inner_ptr = std::get_if<basedparse::Pointer_type_expression>(
    &array8->element_type->value
  );
  REQUIRE(inner_ptr != nullptr);
  REQUIRE(inner_ptr->kw_mut.has_value());
  // [4]
  auto const array4 = std::get_if<basedparse::Array_type_expression>(
    &inner_ptr->pointee_type->value
  );
  REQUIRE(array4 != nullptr);
  auto const size4 =
    std::get_if<basedparse::Int_literal_expression>(&array4->size->value);
  REQUIRE(size4 != nullptr);
  CHECK(size4->literal.text == "4");
  // i32
  auto const elem = std::get_if<basedparse::Identifier_type_expression>(
    &array4->element_type->value
  );
  REQUIRE(elem != nullptr);
  CHECK(elem->identifier.text == "i32");
}

TEST_CASE("parse_type_expression - pointer in full program")
{
  CHECK(parses("let f = fn(x: i32*) -> void { };"));
  CHECK(parses("let f = fn(x: i32 mut*) -> void { };"));
  CHECK(parses("let f = fn(x: i32[]*) -> void { };"));
  CHECK(parses("let f = fn(x: i32[] mut*) -> void { };"));
  CHECK(parses("let f = fn(x: i32[4]*) -> void { };"));
  CHECK(parses("let f = fn(x: i32[4] mut*[8]*) -> void { };"));
}

TEST_CASE("parse_expression - assign: simple")
{
  auto fixture = Parse_fixture{"x = 1"};
  auto const expr = fixture.parser.parse_expression();
  auto const bin = std::get_if<basedparse::Binary_expression>(&expr->value);
  REQUIRE(bin != nullptr);
  CHECK(bin->op.text == "=");
  auto const left = std::get_if<basedparse::Identifier_expression>(&bin->left->value);
  REQUIRE(left != nullptr);
  CHECK(left->identifier.text == "x");
  auto const right = std::get_if<basedparse::Int_literal_expression>(&bin->right->value);
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
  auto const left = std::get_if<basedparse::Identifier_expression>(&outer->left->value);
  REQUIRE(left != nullptr);
  CHECK(left->identifier.text == "a");
  auto const inner = std::get_if<basedparse::Binary_expression>(&outer->right->value);
  REQUIRE(inner != nullptr);
  CHECK(inner->op.text == "=");
  auto const inner_left = std::get_if<basedparse::Identifier_expression>(&inner->left->value);
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
  auto const rhs = std::get_if<basedparse::Binary_expression>(&assign->right->value);
  REQUIRE(rhs != nullptr);
  CHECK(rhs->op.text == "==");
}

TEST_CASE("parse_expression - assign: in full program")
{
  CHECK(parses("let f = fn(mut x: i32) -> void { x = 42; };"));
  CHECK(parses("let f = fn(mut x: i32, mut y: i32) -> void { x = y = 0; };"));
}

TEST_CASE("parse_statement - while: simple")
{
  auto fixture = Parse_fixture{"while x { }"};
  auto const stmt = fixture.parser.parse_statement();
  auto const while_stmt = std::get_if<basedparse::While_statement>(&stmt.value);
  REQUIRE(while_stmt != nullptr);
  CHECK(while_stmt->kw_while.text == "while");
  auto const cond =
    std::get_if<basedparse::Identifier_expression>(&while_stmt->condition->value);
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
    "let f = fn(mut n: i32) -> i32 {"
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
  CHECK(unit->statements.size() == 3);
}
