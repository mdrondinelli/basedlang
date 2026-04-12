#include <type_traits>

#include <catch2/catch_test_macros.hpp>

#include "interning/interning.h"

static_assert(!std::is_move_constructible_v<benson::String_pool>);
static_assert(!std::is_move_assignable_v<benson::String_pool>);

TEST_CASE("String_pool starts with thirty-two buckets")
{
  auto pool = benson::String_pool{};
  CHECK(pool.bucket_count() == 32);
}

TEST_CASE("String_pool rehash updates bucket count")
{
  auto pool = benson::String_pool{};
  pool.rehash(16);
  CHECK(pool.bucket_count() == 16);
}
