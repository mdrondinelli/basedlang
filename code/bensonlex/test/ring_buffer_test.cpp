#include <string>

#include <catch2/catch_test_macros.hpp>

#include "bensonlex/ring_buffer.h"

TEST_CASE("Ring_buffer - push_back and operator[]")
{
  auto buf = benson::Ring_buffer<int>{};
  REQUIRE(buf.size() == 0);
  buf.push_back(1);
  buf.push_back(2);
  buf.push_back(3);
  REQUIRE(buf.size() == 3);
  CHECK(buf[0] == 1);
  CHECK(buf[1] == 2);
  CHECK(buf[2] == 3);
}

TEST_CASE("Ring_buffer - pop_front")
{
  auto buf = benson::Ring_buffer<int>{};
  buf.push_back(1);
  buf.push_back(2);
  buf.push_back(3);
  buf.pop_front();
  REQUIRE(buf.size() == 2);
  CHECK(buf[0] == 2);
  CHECK(buf[1] == 3);
}

TEST_CASE("Ring_buffer - wraparound")
{
  auto buf = benson::Ring_buffer<int>{};
  buf.push_back(1);
  buf.push_back(2);
  buf.push_back(3);
  buf.push_back(4);
  buf.pop_front();
  buf.pop_front();
  buf.push_back(5);
  buf.push_back(6);
  REQUIRE(buf.size() == 4);
  CHECK(buf[0] == 3);
  CHECK(buf[1] == 4);
  CHECK(buf[2] == 5);
  CHECK(buf[3] == 6);
}

TEST_CASE("Ring_buffer - growth")
{
  auto buf = benson::Ring_buffer<int>{};
  buf.push_back(1);
  buf.push_back(2);
  buf.push_back(3);
  buf.push_back(4);
  buf.push_back(5);
  REQUIRE(buf.size() == 5);
  CHECK(buf[0] == 1);
  CHECK(buf[1] == 2);
  CHECK(buf[2] == 3);
  CHECK(buf[3] == 4);
  CHECK(buf[4] == 5);
}

TEST_CASE("Ring_buffer - growth with wrapped head")
{
  auto buf = benson::Ring_buffer<int>{};
  buf.push_back(1);
  buf.push_back(2);
  buf.push_back(3);
  buf.push_back(4);
  buf.pop_front();
  buf.pop_front();
  buf.push_back(5);
  buf.push_back(6);
  buf.push_back(7);
  REQUIRE(buf.size() == 5);
  CHECK(buf[0] == 3);
  CHECK(buf[1] == 4);
  CHECK(buf[2] == 5);
  CHECK(buf[3] == 6);
  CHECK(buf[4] == 7);
}

TEST_CASE("Ring_buffer - move semantics")
{
  auto buf = benson::Ring_buffer<std::string>{};
  buf.push_back("hello");
  buf.push_back("world");
  REQUIRE(buf.size() == 2);
  CHECK(buf[0] == "hello");
  CHECK(buf[1] == "world");
  buf.pop_front();
  REQUIRE(buf.size() == 1);
  CHECK(buf[0] == "world");
}
