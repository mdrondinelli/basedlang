#include <catch2/catch_test_macros.hpp>

#include "kalehlir/constant_value.h"
#include "kalehlir/symbol_table.h"

TEST_CASE("Symbol_table - declare and lookup in global scope")
{
  auto pool = kalehlir::Type_pool{};
  auto table = kalehlir::Symbol_table{};
  auto const sym = table.declare_object("x", pool.int32_type(), false);
  REQUIRE(sym != nullptr);
  CHECK(sym->name == "x");
  auto const ob = std::get_if<kalehlir::Object_binding>(&sym->data);
  REQUIRE(ob != nullptr);
  CHECK(ob->type == pool.int32_type());
  CHECK(ob->is_mutable == false);
  CHECK(table.lookup("x") == sym);
}

TEST_CASE("Symbol_table - declare_value and lookup")
{
  auto pool = kalehlir::Type_pool{};
  auto table = kalehlir::Symbol_table{};
  auto const sym =
    table.declare_value("Int32", kalehlir::Type_value{pool.int32_type()});
  REQUIRE(sym != nullptr);
  auto const cv = std::get_if<kalehlir::Constant_value>(&sym->data);
  REQUIRE(cv != nullptr);
  auto const tv = std::get_if<kalehlir::Type_value>(cv);
  REQUIRE(tv != nullptr);
  CHECK(tv->type == pool.int32_type());
  CHECK(table.lookup("Int32") == sym);
}

TEST_CASE("Symbol_table - lookup returns nullptr for undeclared name")
{
  auto table = kalehlir::Symbol_table{};
  table.push_scope();
  CHECK(table.lookup("x") == nullptr);
}

TEST_CASE("Symbol_table - redeclaration in same scope shadows")
{
  auto pool = kalehlir::Type_pool{};
  auto table = kalehlir::Symbol_table{};
  table.push_scope();
  auto const first = table.declare_object("x", pool.int32_type(), false);
  auto const second = table.declare_object("x", pool.bool_type(), true);
  CHECK(first != second);
  CHECK(table.lookup("x") == second);
}

TEST_CASE("Symbol_table - inner scope shadows outer")
{
  auto pool = kalehlir::Type_pool{};
  auto table = kalehlir::Symbol_table{};
  table.push_scope();
  auto const outer = table.declare_object("x", pool.int32_type(), false);
  table.push_scope();
  auto const inner = table.declare_object("x", pool.bool_type(), true);
  CHECK(inner != outer);
  CHECK(table.lookup("x") == inner);
}

TEST_CASE("Symbol_table - pop scope restores outer symbol")
{
  auto pool = kalehlir::Type_pool{};
  auto table = kalehlir::Symbol_table{};
  table.push_scope();
  auto const outer = table.declare_object("x", pool.int32_type(), false);
  table.push_scope();
  table.declare_object("x", pool.bool_type(), true);
  table.pop_scope();
  CHECK(table.lookup("x") == outer);
}

TEST_CASE("Symbol_table - inner scope finds outer declaration")
{
  auto pool = kalehlir::Type_pool{};
  auto table = kalehlir::Symbol_table{};
  table.push_scope();
  auto const sym = table.declare_object("x", pool.int32_type(), false);
  table.push_scope();
  CHECK(table.lookup("x") == sym);
}

TEST_CASE("Symbol_table - symbols outlive scope pop")
{
  auto pool = kalehlir::Type_pool{};
  auto table = kalehlir::Symbol_table{};
  table.push_scope();
  auto const sym = table.declare_object("x", pool.int32_type(), false);
  table.pop_scope();
  CHECK(sym->name == "x");
  auto const ob = std::get_if<kalehlir::Object_binding>(&sym->data);
  REQUIRE(ob != nullptr);
  CHECK(ob->type == pool.int32_type());
}

TEST_CASE("Symbol_table - barrier blocks lookup of outer locals")
{
  auto pool = kalehlir::Type_pool{};
  auto table = kalehlir::Symbol_table{};
  table.push_scope();
  table.declare_object("x", pool.int32_type(), false);
  table.push_scope(true);
  CHECK(table.lookup("x") == nullptr);
}

TEST_CASE("Symbol_table - barrier does not block globals")
{
  auto pool = kalehlir::Type_pool{};
  auto table = kalehlir::Symbol_table{};
  auto const global = table.declare_object("g", pool.int32_type(), false);
  table.push_scope();
  table.declare_object("x", pool.int32_type(), false);
  table.push_scope(true);
  CHECK(table.lookup("g") == global);
  CHECK(table.lookup("x") == nullptr);
}

TEST_CASE("Symbol_table - declaration inside barrier scope is visible")
{
  auto pool = kalehlir::Type_pool{};
  auto table = kalehlir::Symbol_table{};
  table.push_scope();
  table.push_scope(true);
  auto const sym = table.declare_object("y", pool.bool_type(), true);
  CHECK(table.lookup("y") == sym);
}

TEST_CASE(
  "Symbol_table - local scope inside barrier finds barrier-scope declarations"
)
{
  auto pool = kalehlir::Type_pool{};
  auto table = kalehlir::Symbol_table{};
  table.push_scope();
  table.push_scope(true);
  auto const sym = table.declare_object("p", pool.int32_type(), false);
  table.push_scope();
  CHECK(table.lookup("p") == sym);
}

TEST_CASE("Symbol_table - nested barriers")
{
  auto pool = kalehlir::Type_pool{};
  auto table = kalehlir::Symbol_table{};
  auto const global = table.declare_object("g", pool.int32_type(), false);
  table.push_scope(true);
  auto const outer_fn =
    table.declare_object("outer_local", pool.int32_type(), false);
  table.push_scope(true);
  auto const inner_fn =
    table.declare_object("inner_local", pool.bool_type(), false);
  CHECK(table.lookup("inner_local") == inner_fn);
  CHECK(table.lookup("outer_local") == nullptr);
  CHECK(table.lookup("g") == global);
  static_cast<void>(outer_fn);
}

TEST_CASE("Symbol_table - global value symbols visible through barrier")
{
  auto pool = kalehlir::Type_pool{};
  auto table = kalehlir::Symbol_table{};
  auto const int32_sym =
    table.declare_value("Int32", kalehlir::Type_value{pool.int32_type()});
  table.push_scope(true);
  CHECK(table.lookup("Int32") == int32_sym);
}
