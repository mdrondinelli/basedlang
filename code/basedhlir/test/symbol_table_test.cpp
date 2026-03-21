#include <catch2/catch_test_macros.hpp>

#include "basedhlir/symbol_table.h"

TEST_CASE("Symbol_table - declare and lookup in global scope")
{
  auto pool = basedhlir::Type_pool{};
  auto table = basedhlir::Symbol_table{};
  auto const sym = table.declare_value("x", pool.i32_type(), false);
  REQUIRE(sym != nullptr);
  CHECK(sym->name == "x");
  auto const val = std::get_if<basedhlir::Value_symbol>(&sym->data);
  REQUIRE(val != nullptr);
  CHECK(val->type == pool.i32_type());
  CHECK(val->is_mutable == false);
  CHECK(table.lookup("x") == sym);
}

TEST_CASE("Symbol_table - declare_type and lookup")
{
  auto pool = basedhlir::Type_pool{};
  auto table = basedhlir::Symbol_table{};
  auto const sym = table.declare_type("i32", pool.i32_type());
  REQUIRE(sym != nullptr);
  auto const ts = std::get_if<basedhlir::Type_symbol>(&sym->data);
  REQUIRE(ts != nullptr);
  CHECK(ts->type == pool.i32_type());
  CHECK(table.lookup("i32") == sym);
}

TEST_CASE("Symbol_table - lookup returns nullptr for undeclared name")
{
  auto table = basedhlir::Symbol_table{};
  table.push_scope();
  CHECK(table.lookup("x") == nullptr);
}

TEST_CASE("Symbol_table - redeclaration in same scope shadows")
{
  auto pool = basedhlir::Type_pool{};
  auto table = basedhlir::Symbol_table{};
  table.push_scope();
  auto const first = table.declare_value("x", pool.i32_type(), false);
  auto const second = table.declare_value("x", pool.bool_type(), true);
  CHECK(first != second);
  CHECK(table.lookup("x") == second);
}

TEST_CASE("Symbol_table - inner scope shadows outer")
{
  auto pool = basedhlir::Type_pool{};
  auto table = basedhlir::Symbol_table{};
  table.push_scope();
  auto const outer = table.declare_value("x", pool.i32_type(), false);
  table.push_scope();
  auto const inner = table.declare_value("x", pool.bool_type(), true);
  CHECK(inner != outer);
  CHECK(table.lookup("x") == inner);
}

TEST_CASE("Symbol_table - pop scope restores outer symbol")
{
  auto pool = basedhlir::Type_pool{};
  auto table = basedhlir::Symbol_table{};
  table.push_scope();
  auto const outer = table.declare_value("x", pool.i32_type(), false);
  table.push_scope();
  table.declare_value("x", pool.bool_type(), true);
  table.pop_scope();
  CHECK(table.lookup("x") == outer);
}

TEST_CASE("Symbol_table - inner scope finds outer declaration")
{
  auto pool = basedhlir::Type_pool{};
  auto table = basedhlir::Symbol_table{};
  table.push_scope();
  auto const sym = table.declare_value("x", pool.i32_type(), false);
  table.push_scope();
  CHECK(table.lookup("x") == sym);
}

TEST_CASE("Symbol_table - symbols outlive scope pop")
{
  auto pool = basedhlir::Type_pool{};
  auto table = basedhlir::Symbol_table{};
  table.push_scope();
  auto const sym = table.declare_value("x", pool.i32_type(), false);
  table.pop_scope();
  CHECK(sym->name == "x");
  auto const val = std::get_if<basedhlir::Value_symbol>(&sym->data);
  REQUIRE(val != nullptr);
  CHECK(val->type == pool.i32_type());
}

TEST_CASE("Symbol_table - barrier blocks lookup of outer locals")
{
  auto pool = basedhlir::Type_pool{};
  auto table = basedhlir::Symbol_table{};
  table.push_scope();
  table.declare_value("x", pool.i32_type(), false);
  table.push_scope(true);
  CHECK(table.lookup("x") == nullptr);
}

TEST_CASE("Symbol_table - barrier does not block globals")
{
  auto pool = basedhlir::Type_pool{};
  auto table = basedhlir::Symbol_table{};
  auto const global = table.declare_value("g", pool.i32_type(), false);
  table.push_scope();
  table.declare_value("x", pool.i32_type(), false);
  table.push_scope(true);
  CHECK(table.lookup("g") == global);
  CHECK(table.lookup("x") == nullptr);
}

TEST_CASE("Symbol_table - declaration inside barrier scope is visible")
{
  auto pool = basedhlir::Type_pool{};
  auto table = basedhlir::Symbol_table{};
  table.push_scope();
  table.push_scope(true);
  auto const sym = table.declare_value("y", pool.bool_type(), true);
  CHECK(table.lookup("y") == sym);
}

TEST_CASE("Symbol_table - local scope inside barrier finds barrier-scope declarations")
{
  auto pool = basedhlir::Type_pool{};
  auto table = basedhlir::Symbol_table{};
  table.push_scope();
  table.push_scope(true);
  auto const sym = table.declare_value("p", pool.i32_type(), false);
  table.push_scope();
  CHECK(table.lookup("p") == sym);
}

TEST_CASE("Symbol_table - nested barriers")
{
  auto pool = basedhlir::Type_pool{};
  auto table = basedhlir::Symbol_table{};
  auto const global = table.declare_value("g", pool.i32_type(), false);
  table.push_scope(true);
  auto const outer_fn = table.declare_value("outer_local", pool.i32_type(), false);
  table.push_scope(true);
  auto const inner_fn = table.declare_value("inner_local", pool.bool_type(), false);
  CHECK(table.lookup("inner_local") == inner_fn);
  CHECK(table.lookup("outer_local") == nullptr);
  CHECK(table.lookup("g") == global);
  static_cast<void>(outer_fn);
}

TEST_CASE("Symbol_table - global type symbols visible through barrier")
{
  auto pool = basedhlir::Type_pool{};
  auto table = basedhlir::Symbol_table{};
  auto const i32_sym = table.declare_type("i32", pool.i32_type());
  table.push_scope(true);
  CHECK(table.lookup("i32") == i32_sym);
}
