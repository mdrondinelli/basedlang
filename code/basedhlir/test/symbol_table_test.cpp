#include <catch2/catch_test_macros.hpp>

#include "basedhlir/symbol_table.h"

TEST_CASE("Symbol_table - declare_value and lookup")
{
  auto pool = basedhlir::Type_pool{};
  auto table = basedhlir::Symbol_table{};
  table.push_scope();
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
  table.push_scope();
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
  table.push_scope();
  auto const sym = table.declare_value("x", pool.i32_type(), false);
  table.pop_scope();
  CHECK(sym->name == "x");
  auto const val = std::get_if<basedhlir::Value_symbol>(&sym->data);
  REQUIRE(val != nullptr);
  CHECK(val->type == pool.i32_type());
}
