#include <cstddef>
#include <cstdint>
#include <cstring>

#include <catch2/catch_test_macros.hpp>

#include "bytecode/module.h"

TEST_CASE("Module - default constructed is empty")
{
  auto const m = benson::bytecode::Module{};
  CHECK(m.code.empty());
  CHECK(m.constant_data.empty());
  CHECK(m.constant_table.empty());
}

TEST_CASE("Module - code bytes are accessible")
{
  auto m = benson::bytecode::Module{};
  m.code.push_back(std::byte{0x01});
  m.code.push_back(std::byte{0x02});
  REQUIRE(m.code.size() == 2);
  CHECK(m.code[0] == std::byte{0x01});
  CHECK(m.code[1] == std::byte{0x02});
}

TEST_CASE("Module - constant data and table are accessible")
{
  auto m = benson::bytecode::Module{};
  auto const value = std::int32_t{42};
  auto const offset = static_cast<std::ptrdiff_t>(m.constant_data.size());
  m.constant_data.resize(m.constant_data.size() + sizeof(value));
  std::memcpy(m.constant_data.data() + offset, &value, sizeof(value));
  m.constant_table.push_back(offset);
  REQUIRE(m.constant_table.size() == 1);
  CHECK(m.constant_table[0] == offset);
  auto read_back = std::int32_t{};
  std::memcpy(&read_back, m.constant_data.data() + m.constant_table[0], sizeof(read_back));
  CHECK(read_back == value);
}
