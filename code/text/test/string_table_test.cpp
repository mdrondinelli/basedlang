#include <type_traits>
#include <unordered_set>

#include <catch2/catch_test_macros.hpp>

#include "text/string_table.h"

static_assert(!std::is_move_constructible_v<benson::String_table>);
static_assert(!std::is_move_assignable_v<benson::String_table>);

TEST_CASE("String_table interns identical spellings into one row")
{
  auto table = benson::String_table{};
  auto foo = table.intern(U"foo");
  auto foo_again = table.intern(U"foo");
  CHECK(foo == foo_again);
  CHECK(foo.view() == U"foo");
  CHECK(table.row_count() == 1);
}

TEST_CASE("String_table keeps distinct spellings in distinct rows")
{
  auto table = benson::String_table{};
  auto foo = table.intern(U"foo");
  auto bar = table.intern(U"bar");
  CHECK(foo != bar);
  CHECK(foo.view() == U"foo");
  CHECK(bar.view() == U"bar");
  CHECK(table.row_count() == 2);
}

TEST_CASE("Interned_string clone keeps the row alive")
{
  auto table = benson::String_table{};
  auto foo = table.intern(U"foo");
  auto foo_clone = foo.clone();
  CHECK(foo == foo_clone);
  foo = {};
  CHECK(foo.empty());
  CHECK(foo_clone.view() == U"foo");
  CHECK(table.row_count() == 1);
}

TEST_CASE("Interned_string move transfers ownership")
{
  auto table = benson::String_table{};
  auto foo = table.intern(U"foo");
  auto moved = std::move(foo);
  CHECK(foo.empty());
  CHECK(moved.view() == U"foo");
  CHECK(table.row_count() == 1);
}

TEST_CASE("String_table reclaims rows when the last owner releases")
{
  auto table = benson::String_table{};
  {
    auto foo = table.intern(U"foo");
    auto foo_clone = foo.clone();
    CHECK(table.row_count() == 1);
    foo = {};
    CHECK(table.row_count() == 1);
    foo_clone = {};
  }
  CHECK(table.row_count() == 0);
}

TEST_CASE("String_table can re-intern a spelling after reclamation")
{
  auto table = benson::String_table{};
  {
    auto foo = table.intern(U"foo");
    CHECK(table.row_count() == 1);
  }
  CHECK(table.row_count() == 0);
  auto foo = table.intern(U"foo");
  CHECK(foo.view() == U"foo");
  CHECK(table.row_count() == 1);
}

TEST_CASE("Interned_string hashing follows handle equality")
{
  auto table = benson::String_table{};
  auto foo = table.intern(U"foo");
  auto foo_clone = foo.clone();
  auto bar = table.intern(U"bar");
  auto set = std::unordered_set<benson::Interned_string>{};
  set.insert(std::move(foo));
  set.insert(std::move(foo_clone));
  set.insert(std::move(bar));
  CHECK(set.size() == 2);
}

TEST_CASE("Interned_string handles moved-from and empty states safely")
{
  auto empty = benson::Interned_string{};
  CHECK(empty.empty());
  CHECK(empty.view().empty());
  auto table = benson::String_table{};
  auto foo = table.intern(U"foo");
  auto moved = std::move(foo);
  CHECK(foo.empty());
  CHECK(foo.view().empty());
  CHECK(moved);
}

TEST_CASE("String_table preserves non-ASCII codepoints")
{
  auto table = benson::String_table{};
  auto snowman = table.intern(U"\u2603");
  auto mixed = table.intern(U"\u03bbx");
  CHECK(snowman.view() == U"\u2603");
  CHECK(mixed.view() == U"\u03bbx");
  CHECK(table.row_count() == 2);
}
