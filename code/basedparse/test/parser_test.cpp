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
  REQUIRE(fn_def->function.return_type_specifier.has_value());
  auto const *return_type =
    dynamic_cast<basedparse::Identifier_type_expression const *>(
      fn_def->function.return_type_specifier->type_expression.get()
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

TEST_CASE("Parser - parameters.based parses successfully")
{
  auto file =
    std::ifstream{std::string{EXAMPLES_PATH} + "/parameters.based"};
  auto binary_stream = basedlex::Istream_binary_stream{&file};
  auto char_stream = basedlex::Utf8_char_stream{&binary_stream};
  auto lexeme_stream = basedlex::Lexeme_stream{&char_stream};
  auto reader = basedlex::Lexeme_stream_reader{&lexeme_stream};
  auto parser = basedparse::Parser{&reader};
  auto const unit = parser.parse_translation_unit();
  REQUIRE(unit->statements.size() == 3);
  auto const *id_def = dynamic_cast<basedparse::Function_definition const *>(
    unit->statements[0].get()
  );
  auto const *first_def = dynamic_cast<basedparse::Function_definition const *>(
    unit->statements[1].get()
  );
  auto const *main_def = dynamic_cast<basedparse::Function_definition const *>(
    unit->statements[2].get()
  );
  REQUIRE(id_def != nullptr);
  REQUIRE(first_def != nullptr);
  REQUIRE(main_def != nullptr);
  CHECK(id_def->name.text == "id");
  CHECK(first_def->name.text == "first");
  CHECK(main_def->name.text == "main");
  // id: fn(x: i32) -> i32 { return x; }
  REQUIRE(id_def->function.parameters.size() == 1);
  CHECK(id_def->function.parameters[0].name.text == "x");
  auto const *id_param_type =
    dynamic_cast<basedparse::Identifier_type_expression const *>(
      id_def->function.parameters[0].type_expression.get()
    );
  REQUIRE(id_param_type != nullptr);
  CHECK(id_param_type->identifier.text == "i32");
  REQUIRE(id_def->function.return_type_specifier.has_value());
  auto const *id_ret_type =
    dynamic_cast<basedparse::Identifier_type_expression const *>(
      id_def->function.return_type_specifier->type_expression.get()
    );
  REQUIRE(id_ret_type != nullptr);
  CHECK(id_ret_type->identifier.text == "i32");
  REQUIRE(id_def->function.body->statements.size() == 1);
  auto const *id_ret = dynamic_cast<basedparse::Return_statement const *>(
    id_def->function.body->statements[0].get()
  );
  REQUIRE(id_ret != nullptr);
  auto const *id_ret_val =
    dynamic_cast<basedparse::Identifier_expression const *>(
      id_ret->value.get()
    );
  REQUIRE(id_ret_val != nullptr);
  CHECK(id_ret_val->identifier.text == "x");
  // first: fn(x: i32, y: i32) -> i32 { return x; }
  REQUIRE(first_def->function.parameters.size() == 2);
  CHECK(first_def->function.parameters[0].name.text == "x");
  auto const *first_param0_type =
    dynamic_cast<basedparse::Identifier_type_expression const *>(
      first_def->function.parameters[0].type_expression.get()
    );
  REQUIRE(first_param0_type != nullptr);
  CHECK(first_param0_type->identifier.text == "i32");
  CHECK(first_def->function.parameters[1].name.text == "y");
  auto const *first_param1_type =
    dynamic_cast<basedparse::Identifier_type_expression const *>(
      first_def->function.parameters[1].type_expression.get()
    );
  REQUIRE(first_param1_type != nullptr);
  CHECK(first_param1_type->identifier.text == "i32");
  REQUIRE(first_def->function.return_type_specifier.has_value());
  auto const *first_ret_type =
    dynamic_cast<basedparse::Identifier_type_expression const *>(
      first_def->function.return_type_specifier->type_expression.get()
    );
  REQUIRE(first_ret_type != nullptr);
  CHECK(first_ret_type->identifier.text == "i32");
  REQUIRE(first_def->function.body->statements.size() == 1);
  auto const *first_ret = dynamic_cast<basedparse::Return_statement const *>(
    first_def->function.body->statements[0].get()
  );
  REQUIRE(first_ret != nullptr);
  auto const *first_ret_val =
    dynamic_cast<basedparse::Identifier_expression const *>(
      first_ret->value.get()
    );
  REQUIRE(first_ret_val != nullptr);
  CHECK(first_ret_val->identifier.text == "x");
  // main: fn() -> i32 { return first(id(42), 0); }
  REQUIRE(main_def->function.return_type_specifier.has_value());
  auto const *main_ret_type =
    dynamic_cast<basedparse::Identifier_type_expression const *>(
      main_def->function.return_type_specifier->type_expression.get()
    );
  REQUIRE(main_ret_type != nullptr);
  CHECK(main_ret_type->identifier.text == "i32");
  REQUIRE(main_def->function.body->statements.size() == 1);
  auto const *main_ret = dynamic_cast<basedparse::Return_statement const *>(
    main_def->function.body->statements[0].get()
  );
  REQUIRE(main_ret != nullptr);
  // first(id(42), 0) — outer call
  auto const *outer_call =
    dynamic_cast<basedparse::Call_expression const *>(main_ret->value.get());
  REQUIRE(outer_call != nullptr);
  auto const *outer_callee =
    dynamic_cast<basedparse::Identifier_expression const *>(
      outer_call->callee.get()
    );
  REQUIRE(outer_callee != nullptr);
  CHECK(outer_callee->identifier.text == "first");
  REQUIRE(outer_call->arguments.size() == 2);
  auto const *inner_call =
    dynamic_cast<basedparse::Call_expression const *>(
      outer_call->arguments[0].get()
    );
  REQUIRE(inner_call != nullptr);
  auto const *inner_callee =
    dynamic_cast<basedparse::Identifier_expression const *>(
      inner_call->callee.get()
    );
  REQUIRE(inner_callee != nullptr);
  CHECK(inner_callee->identifier.text == "id");
  REQUIRE(inner_call->arguments.size() == 1);
  auto const *inner_arg =
    dynamic_cast<basedparse::Int_literal_expression const *>(
      inner_call->arguments[0].get()
    );
  REQUIRE(inner_arg != nullptr);
  CHECK(inner_arg->literal.text == "42");
  auto const *outer_arg1 =
    dynamic_cast<basedparse::Int_literal_expression const *>(
      outer_call->arguments[1].get()
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
  auto const *foo = dynamic_cast<basedparse::Function_definition const *>(
    unit->statements[0].get()
  );
  auto const *main = dynamic_cast<basedparse::Function_definition const *>(
    unit->statements[1].get()
  );
  REQUIRE(foo != nullptr);
  REQUIRE(main != nullptr);
  CHECK(foo->name.text == "foo");
  CHECK(main->name.text == "main");
  // foo returns fn() -> i32 { return 0; }() — a call with an fn expression callee
  REQUIRE(foo->function.body->statements.size() == 1);
  auto const *foo_ret = dynamic_cast<basedparse::Return_statement const *>(
    foo->function.body->statements[0].get()
  );
  REQUIRE(foo_ret != nullptr);
  auto const *foo_call =
    dynamic_cast<basedparse::Call_expression const *>(foo_ret->value.get());
  REQUIRE(foo_call != nullptr);
  CHECK(foo_call->lparen.text == "(");
  CHECK(foo_call->rparen.text == ")");
  CHECK(
    dynamic_cast<basedparse::Fn_expression const *>(foo_call->callee.get()) !=
    nullptr
  );
  // main returns foo() — a call with an identifier callee
  REQUIRE(main->function.body->statements.size() == 1);
  auto const *main_ret = dynamic_cast<basedparse::Return_statement const *>(
    main->function.body->statements[0].get()
  );
  REQUIRE(main_ret != nullptr);
  auto const *main_call =
    dynamic_cast<basedparse::Call_expression const *>(main_ret->value.get());
  REQUIRE(main_call != nullptr);
  CHECK(main_call->lparen.text == "(");
  CHECK(main_call->rparen.text == ")");
  auto const *callee = dynamic_cast<basedparse::Identifier_expression const *>(
    main_call->callee.get()
  );
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
  CHECK_FALSE(parses("let x = fn() -> i32 { return * 1; };"));
  CHECK_FALSE(parses("let x = fn() -> i32 { return 1 + * 2; };"));
  CHECK_FALSE(parses("let x = fn() -> i32 { return +; };"));
  CHECK_FALSE(parses("let x = fn() -> i32 { return -; };"));
  // malformed array types
  CHECK_FALSE(parses("let f = fn(x: i32[]) -> void { };"));
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
  auto fixture = Parse_fixture{"let main = fn() -> i32 { return 0; };"};
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
  REQUIRE(fn->return_type_specifier.has_value());
  auto const *ret_type =
    dynamic_cast<basedparse::Identifier_type_expression const *>(
      fn->return_type_specifier->type_expression.get()
    );
  REQUIRE(ret_type != nullptr);
  CHECK(ret_type->identifier.text == "i32");
  REQUIRE(fn->body != nullptr);
  CHECK(fn->body->statements.size() == 1);
}

TEST_CASE("parse_fn_expression - no return type")
{
  auto fixture = Parse_fixture{"fn() { }"};
  auto const fn = fixture.parser.parse_fn_expression();
  CHECK_FALSE(fn->return_type_specifier.has_value());
  REQUIRE(fn->body != nullptr);
  CHECK(fn->body->statements.empty());
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

TEST_CASE("parse_type_expression - identifier")
{
  auto fixture = Parse_fixture{"i32"};
  auto const type = fixture.parser.parse_type_expression();
  auto const expr =
    dynamic_cast<basedparse::Identifier_type_expression const *>(type.get());
  REQUIRE(expr != nullptr);
  CHECK(expr->identifier.text == "i32");
}

TEST_CASE("parse_paren_expression")
{
  auto fixture = Parse_fixture{"(42)"};
  auto const expr = fixture.parser.parse_paren_expression();
  CHECK(expr->lparen.text == "(");
  auto const inner =
    dynamic_cast<basedparse::Int_literal_expression const *>(expr->inner.get());
  REQUIRE(inner != nullptr);
  CHECK(inner->literal.text == "42");
  CHECK(expr->rparen.text == ")");
}

TEST_CASE("parse_paren_expression - nested")
{
  auto fixture = Parse_fixture{"((x))"};
  auto const expr = fixture.parser.parse_paren_expression();
  CHECK(expr->lparen.text == "(");
  auto const inner =
    dynamic_cast<basedparse::Paren_expression const *>(expr->inner.get());
  REQUIRE(inner != nullptr);
  auto const id =
    dynamic_cast<basedparse::Identifier_expression const *>(inner->inner.get());
  REQUIRE(id != nullptr);
  CHECK(id->identifier.text == "x");
  CHECK(inner->rparen.text == ")");
  CHECK(expr->rparen.text == ")");
}

TEST_CASE("parse_primary_expression - dispatches to int literal")
{
  auto fixture = Parse_fixture{"123"};
  auto const expr = fixture.parser.parse_primary_expression();
  CHECK(
    dynamic_cast<basedparse::Int_literal_expression const *>(expr.get()) !=
    nullptr
  );
}

TEST_CASE("parse_primary_expression - dispatches to identifier")
{
  auto fixture = Parse_fixture{"x"};
  auto const expr = fixture.parser.parse_primary_expression();
  CHECK(
    dynamic_cast<basedparse::Identifier_expression const *>(expr.get()) !=
    nullptr
  );
}

TEST_CASE("parse_primary_expression - dispatches to fn")
{
  auto fixture = Parse_fixture{"fn() -> i32 { }"};
  auto const expr = fixture.parser.parse_primary_expression();
  CHECK(dynamic_cast<basedparse::Fn_expression const *>(expr.get()) != nullptr);
}

TEST_CASE("parse_primary_expression - dispatches to paren")
{
  auto fixture = Parse_fixture{"(42)"};
  auto const expr = fixture.parser.parse_primary_expression();
  CHECK(
    dynamic_cast<basedparse::Paren_expression const *>(expr.get()) != nullptr
  );
}

TEST_CASE("parse_expression - simple binary expression")
{
  auto fixture = Parse_fixture{"1 + 2"};
  auto const expr = fixture.parser.parse_expression();
  auto const *bin =
    dynamic_cast<basedparse::Binary_expression const *>(expr.get());
  REQUIRE(bin != nullptr);
  auto const *left =
    dynamic_cast<basedparse::Int_literal_expression const *>(bin->left.get());
  REQUIRE(left != nullptr);
  CHECK(left->literal.text == "1");
  CHECK(bin->op.text == "+");
  auto const *right =
    dynamic_cast<basedparse::Int_literal_expression const *>(bin->right.get());
  REQUIRE(right != nullptr);
  CHECK(right->literal.text == "2");
}

TEST_CASE("parse_expression - multiplicative before additive")
{
  // 1 + 2 * 3 should parse as 1 + (2 * 3)
  auto fixture = Parse_fixture{"1 + 2 * 3"};
  auto const expr = fixture.parser.parse_expression();
  auto const *add =
    dynamic_cast<basedparse::Binary_expression const *>(expr.get());
  REQUIRE(add != nullptr);
  CHECK(add->op.text == "+");
  auto const *mul =
    dynamic_cast<basedparse::Binary_expression const *>(add->right.get());
  REQUIRE(mul != nullptr);
  CHECK(mul->op.text == "*");
  auto const *two =
    dynamic_cast<basedparse::Int_literal_expression const *>(mul->left.get());
  REQUIRE(two != nullptr);
  CHECK(two->literal.text == "2");
  auto const *three =
    dynamic_cast<basedparse::Int_literal_expression const *>(mul->right.get());
  REQUIRE(three != nullptr);
  CHECK(three->literal.text == "3");
}

TEST_CASE("parse_expression - left associativity")
{
  // 1 - 2 - 3 should parse as (1 - 2) - 3
  auto fixture = Parse_fixture{"1 - 2 - 3"};
  auto const expr = fixture.parser.parse_expression();
  auto const *outer =
    dynamic_cast<basedparse::Binary_expression const *>(expr.get());
  REQUIRE(outer != nullptr);
  CHECK(outer->op.text == "-");
  auto const *inner =
    dynamic_cast<basedparse::Binary_expression const *>(outer->left.get());
  REQUIRE(inner != nullptr);
  CHECK(inner->op.text == "-");
  auto const *three =
    dynamic_cast<basedparse::Int_literal_expression const *>(outer->right.get()
    );
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
  auto const *sub =
    dynamic_cast<basedparse::Binary_expression const *>(expr.get());
  REQUIRE(sub != nullptr);
  CHECK(sub->op.text == "-");
  // left of -: -f() + +2
  auto const *add =
    dynamic_cast<basedparse::Binary_expression const *>(sub->left.get());
  REQUIRE(add != nullptr);
  CHECK(add->op.text == "+");
  // left of +: -f()
  auto const *unary_minus =
    dynamic_cast<basedparse::Unary_expression const *>(add->left.get());
  REQUIRE(unary_minus != nullptr);
  CHECK(unary_minus->op.text == "-");
  auto const *call =
    dynamic_cast<basedparse::Call_expression const *>(
      unary_minus->operand.get()
    );
  REQUIRE(call != nullptr);
  auto const *callee =
    dynamic_cast<basedparse::Identifier_expression const *>(call->callee.get());
  REQUIRE(callee != nullptr);
  CHECK(callee->identifier.text == "f");
  // right of +: +2
  auto const *unary_plus =
    dynamic_cast<basedparse::Unary_expression const *>(add->right.get());
  REQUIRE(unary_plus != nullptr);
  CHECK(unary_plus->op.text == "+");
  // right of -: ((3 * 4) / 5) % 6
  auto const *mod =
    dynamic_cast<basedparse::Binary_expression const *>(sub->right.get());
  REQUIRE(mod != nullptr);
  CHECK(mod->op.text == "%");
  // left of %: (3 * 4) / 5
  auto const *div =
    dynamic_cast<basedparse::Binary_expression const *>(mod->left.get());
  REQUIRE(div != nullptr);
  CHECK(div->op.text == "/");
  // left of /: 3 * 4
  auto const *mul =
    dynamic_cast<basedparse::Binary_expression const *>(div->left.get());
  REQUIRE(mul != nullptr);
  CHECK(mul->op.text == "*");
}

TEST_CASE("parse_expression - call binds tighter than binary op")
{
  // f() + 1 should parse as (f()) + 1
  auto fixture = Parse_fixture{"f() + 1"};
  auto const expr = fixture.parser.parse_expression();
  auto const *add =
    dynamic_cast<basedparse::Binary_expression const *>(expr.get());
  REQUIRE(add != nullptr);
  CHECK(add->op.text == "+");
  CHECK(
    dynamic_cast<basedparse::Call_expression const *>(add->left.get()) != nullptr
  );
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

TEST_CASE("parse_expression - unary minus: call binds tighter")
{
  // -f() should parse as -(f()), not (-f)()
  auto fixture = Parse_fixture{"-f()"};
  auto const expr = fixture.parser.parse_expression();
  auto const *unary =
    dynamic_cast<basedparse::Unary_expression const *>(expr.get());
  REQUIRE(unary != nullptr);
  CHECK(unary->op.text == "-");
  CHECK(
    dynamic_cast<basedparse::Call_expression const *>(unary->operand.get()) !=
    nullptr
  );
}

TEST_CASE("parse_type_expression - array")
{
  auto fixture = Parse_fixture{"i32[4]"};
  auto const type = fixture.parser.parse_type_expression();
  auto const *array =
    dynamic_cast<basedparse::Array_type_expression const *>(type.get());
  REQUIRE(array != nullptr);
  auto const *elem =
    dynamic_cast<basedparse::Identifier_type_expression const *>(
      array->element_type.get()
    );
  REQUIRE(elem != nullptr);
  CHECK(elem->identifier.text == "i32");
  auto const *size =
    dynamic_cast<basedparse::Int_literal_expression const *>(
      array->size.get()
    );
  REQUIRE(size != nullptr);
  CHECK(size->literal.text == "4");
}

TEST_CASE("parse_type_expression - array with expression size")
{
  auto fixture = Parse_fixture{"i32[n]"};
  auto const type = fixture.parser.parse_type_expression();
  auto const *array =
    dynamic_cast<basedparse::Array_type_expression const *>(type.get());
  REQUIRE(array != nullptr);
  auto const *size =
    dynamic_cast<basedparse::Identifier_expression const *>(array->size.get());
  REQUIRE(size != nullptr);
  CHECK(size->identifier.text == "n");
}

TEST_CASE("parse_type_expression - nested array")
{
  // i32[4][2] parses as (i32[4])[2]
  auto fixture = Parse_fixture{"i32[4][2]"};
  auto const type = fixture.parser.parse_type_expression();
  auto const *outer =
    dynamic_cast<basedparse::Array_type_expression const *>(type.get());
  REQUIRE(outer != nullptr);
  auto const *outer_size =
    dynamic_cast<basedparse::Int_literal_expression const *>(
      outer->size.get()
    );
  REQUIRE(outer_size != nullptr);
  CHECK(outer_size->literal.text == "2");
  auto const *inner =
    dynamic_cast<basedparse::Array_type_expression const *>(
      outer->element_type.get()
    );
  REQUIRE(inner != nullptr);
  auto const *inner_size =
    dynamic_cast<basedparse::Int_literal_expression const *>(
      inner->size.get()
    );
  REQUIRE(inner_size != nullptr);
  CHECK(inner_size->literal.text == "4");
  auto const *elem =
    dynamic_cast<basedparse::Identifier_type_expression const *>(
      inner->element_type.get()
    );
  REQUIRE(elem != nullptr);
  CHECK(elem->identifier.text == "i32");
}

TEST_CASE("parse_fn_expression - array parameter")
{
  auto fixture = Parse_fixture{"fn(buf: i32[4]) { }"};
  auto const fn = fixture.parser.parse_fn_expression();
  REQUIRE(fn->parameters.size() == 1);
  CHECK(fn->parameters[0].name.text == "buf");
  auto const *param_type =
    dynamic_cast<basedparse::Array_type_expression const *>(
      fn->parameters[0].type_expression.get()
    );
  REQUIRE(param_type != nullptr);
  auto const *elem =
    dynamic_cast<basedparse::Identifier_type_expression const *>(
      param_type->element_type.get()
    );
  REQUIRE(elem != nullptr);
  CHECK(elem->identifier.text == "i32");
  auto const *size =
    dynamic_cast<basedparse::Int_literal_expression const *>(
      param_type->size.get()
    );
  REQUIRE(size != nullptr);
  CHECK(size->literal.text == "4");
}


TEST_CASE("parse_expression - constructor: empty")
{
  auto fixture = Parse_fixture{"Foo{}"};
  auto const expr = fixture.parser.parse_expression();
  auto const *ctor =
    dynamic_cast<basedparse::Constructor_expression const *>(expr.get());
  REQUIRE(ctor != nullptr);
  auto const *type =
    dynamic_cast<basedparse::Identifier_type_expression const *>(
      ctor->type.get()
    );
  REQUIRE(type != nullptr);
  CHECK(type->identifier.text == "Foo");
  CHECK(ctor->arguments.empty());
}

TEST_CASE("parse_expression - constructor: multiple arguments")
{
  auto fixture = Parse_fixture{"Foo{1, x, 2 + 3}"};
  auto const expr = fixture.parser.parse_expression();
  auto const *ctor =
    dynamic_cast<basedparse::Constructor_expression const *>(expr.get());
  REQUIRE(ctor != nullptr);
  REQUIRE(ctor->arguments.size() == 3);
  auto const *first =
    dynamic_cast<basedparse::Int_literal_expression const *>(
      ctor->arguments[0].get()
    );
  REQUIRE(first != nullptr);
  CHECK(first->literal.text == "1");
  auto const *second =
    dynamic_cast<basedparse::Identifier_expression const *>(
      ctor->arguments[1].get()
    );
  REQUIRE(second != nullptr);
  CHECK(second->identifier.text == "x");
  CHECK(
    dynamic_cast<basedparse::Binary_expression const *>(
      ctor->arguments[2].get()
    ) != nullptr
  );
}

TEST_CASE("parse_expression - constructor: array type")
{
  auto fixture = Parse_fixture{"i32[4] { 1, 2, 3, 4 }"};
  auto const expr = fixture.parser.parse_expression();
  auto const *ctor =
    dynamic_cast<basedparse::Constructor_expression const *>(expr.get());
  REQUIRE(ctor != nullptr);
  auto const *type =
    dynamic_cast<basedparse::Array_type_expression const *>(ctor->type.get());
  REQUIRE(type != nullptr);
  auto const *elem =
    dynamic_cast<basedparse::Identifier_type_expression const *>(
      type->element_type.get()
    );
  REQUIRE(elem != nullptr);
  CHECK(elem->identifier.text == "i32");
  REQUIRE(ctor->arguments.size() == 4);
}

TEST_CASE("parse_expression - constructor: in full program")
{
  CHECK(parses("let f = fn() { let x = Foo{1, 2}; };"));
  CHECK(parses("let f = fn() { let x = i32[3] { 1, 2, 3 }; };"));
}

TEST_CASE("parse_expression - identifier still works after constructor change")
{
  // plain identifiers must not be misinterpreted as constructors
  auto fixture = Parse_fixture{"x + y"};
  auto const expr = fixture.parser.parse_expression();
  auto const *bin =
    dynamic_cast<basedparse::Binary_expression const *>(expr.get());
  REQUIRE(bin != nullptr);
  auto const *left =
    dynamic_cast<basedparse::Identifier_expression const *>(bin->left.get());
  REQUIRE(left != nullptr);
  CHECK(left->identifier.text == "x");
  auto const *right =
    dynamic_cast<basedparse::Identifier_expression const *>(bin->right.get());
  REQUIRE(right != nullptr);
  CHECK(right->identifier.text == "y");
}

TEST_CASE("parse_expression - constructor: trailing comma")
{
  auto fixture = Parse_fixture{"Foo{1, 2,}"};
  auto const expr = fixture.parser.parse_expression();
  auto const *ctor =
    dynamic_cast<basedparse::Constructor_expression const *>(expr.get());
  REQUIRE(ctor != nullptr);
  REQUIRE(ctor->arguments.size() == 2);
  CHECK(ctor->argument_commas.size() == 2);
}

TEST_CASE("parse_expression - constructor: rejects")
{
  CHECK_FALSE(parses("let x = fn() { Foo{;}; };"));
  CHECK_FALSE(parses("let x = fn() { Foo{1 2}; };"));
  CHECK_FALSE(parses("let x = fn() { i32[4] ; };"));
}
