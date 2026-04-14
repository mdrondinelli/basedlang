#include <string>
#include <string_view>
#include <type_traits>
#include <utility>

#include <catch2/catch_test_macros.hpp>

#include "spelling/spelling.h"

namespace
{

  auto spell(benson::Spelling_table &table, std::string_view text)
    -> benson::Spelling
  {
    auto builder = table.builder();
    builder.append(text);
    return std::move(builder).finalize();
  }

} // namespace

static_assert(!std::is_copy_constructible_v<benson::Spelling_table>);
static_assert(!std::is_copy_assignable_v<benson::Spelling_table>);
static_assert(!std::is_copy_constructible_v<benson::Spelling_table::Builder>);
static_assert(!std::is_copy_assignable_v<benson::Spelling_table::Builder>);

TEST_CASE("Spelling_table starts with thirty-two buckets")
{
  auto table = benson::Spelling_table{};
  CHECK(table.bucket_count() == 32);
  CHECK(table.size() == 0);
}

TEST_CASE("equal spellings deduplicate to the same handle")
{
  auto table = benson::Spelling_table{};
  auto const first = spell(table, "main");
  auto const second = spell(table, "main");
  CHECK(first.has_value());
  CHECK(first == second);
  CHECK(table.size() == 1);
  CHECK(table.lookup(first) == "main");
}

TEST_CASE("different spellings produce distinct handles")
{
  auto table = benson::Spelling_table{};
  auto const first = spell(table, "main");
  auto const second = spell(table, "helper");
  CHECK(first != second);
  CHECK(table.size() == 2);
  CHECK(table.lookup(first) == "main");
  CHECK(table.lookup(second) == "helper");
}

TEST_CASE("builder can append from chars and string views")
{
  auto table = benson::Spelling_table{};
  auto builder = table.builder();
  builder.append('m');
  builder.append("ai");
  builder.append('n');
  auto const spelling = std::move(builder).finalize();
  CHECK(table.lookup(spelling) == "main");
  CHECK(table.size() == 1);
}

TEST_CASE("destroying an unfinished builder rolls back the build")
{
  auto table = benson::Spelling_table{};
  {
    auto builder = table.builder();
    builder.append("temporary");
  }
  auto const spelling = spell(table, "stable");
  CHECK(table.size() == 1);
  CHECK(table.lookup(spelling) == "stable");
}

TEST_CASE("moving a builder preserves the in-progress spelling")
{
  auto table = benson::Spelling_table{};
  auto first = table.builder();
  first.append("he");
  auto second = std::move(first);
  second.append("llo");
  auto const spelling = std::move(second).finalize();
  CHECK(table.lookup(spelling) == "hello");
}

TEST_CASE("empty spellings deduplicate and remain addressable")
{
  auto table = benson::Spelling_table{};
  auto first = table.builder();
  auto const empty0 = std::move(first).finalize();
  auto second = table.builder();
  auto const empty1 = std::move(second).finalize();
  CHECK(empty0.has_value());
  CHECK(empty0 == empty1);
  CHECK(table.lookup(empty0).empty());
  CHECK(table.size() == 1);
}

TEST_CASE("rehash preserves stored spellings and handle lookup")
{
  auto table = benson::Spelling_table{};
  auto first = benson::Spelling{};
  auto last = benson::Spelling{};
  for (auto i = 0; i < 64; ++i)
  {
    auto builder = table.builder();
    builder.append("name");
    builder.append(std::to_string(i));
    auto const spelling = std::move(builder).finalize();
    if (i == 0)
    {
      first = spelling;
    }
    last = spelling;
  }
  CHECK(table.bucket_count() > 32);
  CHECK(table.lookup(first) == "name0");
  CHECK(table.lookup(last) == "name63");
  CHECK(table.size() == 64);
}
