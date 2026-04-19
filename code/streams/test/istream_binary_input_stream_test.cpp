#include <algorithm>
#include <array>
#include <sstream>
#include <vector>

#include <catch2/catch_test_macros.hpp>

#include "streams/binary_input_stream.h"
#include "streams/istream_binary_input_stream.h"

TEST_CASE("Istream_binary_input_stream - read_bytes")
{
  SECTION("empty stream returns 0")
  {
    auto ss = std::istringstream{""};
    auto binary = benson::Istream_binary_input_stream{&ss};
    auto buffer = std::array<uint8_t, 4>{};
    REQUIRE(binary.read_bytes(buffer) == 0);
  }
  SECTION("empty buffer is no-op")
  {
    auto ss = std::istringstream{"hello"};
    auto binary = benson::Istream_binary_input_stream{&ss};
    auto empty = std::span<uint8_t>{};
    auto buffer = std::array<uint8_t, 5>{};
    REQUIRE(binary.read_bytes(empty) == 0);
    REQUIRE(binary.read_bytes(buffer) == 5);
    CHECK(buffer[0] == 'h');
    CHECK(buffer[1] == 'e');
    CHECK(buffer[2] == 'l');
    CHECK(buffer[3] == 'l');
    CHECK(buffer[4] == 'o');
  }
  SECTION("fills full buffer when enough bytes exist")
  {
    auto ss = std::istringstream{"hello"};
    auto binary = benson::Istream_binary_input_stream{&ss};
    auto buffer = std::array<uint8_t, 4>{};
    REQUIRE(binary.read_bytes(buffer) == 4);
    CHECK(buffer[0] == 'h');
    CHECK(buffer[1] == 'e');
    CHECK(buffer[2] == 'l');
    CHECK(buffer[3] == 'l');
  }
  SECTION("short final read returns remaining bytes")
  {
    auto ss = std::istringstream{"hello"};
    auto binary = benson::Istream_binary_input_stream{&ss};
    auto buffer = std::array<uint8_t, 8>{};
    REQUIRE(binary.read_bytes(buffer) == 5);
    CHECK(buffer[0] == 'h');
    CHECK(buffer[1] == 'e');
    CHECK(buffer[2] == 'l');
    CHECK(buffer[3] == 'l');
    CHECK(buffer[4] == 'o');
    REQUIRE(binary.read_bytes(buffer) == 0);
  }
  SECTION("repeated reads reconstruct source")
  {
    auto ss = std::istringstream{"abcdefg"};
    auto binary = benson::Istream_binary_input_stream{&ss};
    auto buffer = std::array<uint8_t, 3>{};
    auto bytes = std::vector<uint8_t>{};
    for (;;)
    {
      auto const count = binary.read_bytes(buffer);
      if (count == 0)
      {
        break;
      }
      bytes.insert(bytes.end(), buffer.begin(), buffer.begin() + count);
    }
    REQUIRE(bytes == std::vector<uint8_t>{'a', 'b', 'c', 'd', 'e', 'f', 'g'});
  }
}

TEST_CASE("Binary_input_stream - read_byte compatibility helper")
{
  class Chunked_binary_input_stream: public benson::Binary_input_stream
  {
  public:
    std::ptrdiff_t read_bytes(std::span<uint8_t> buffer) override
    {
      if (_offset == static_cast<std::ptrdiff_t>(_bytes.size()) ||
          buffer.empty())
      {
        return 0;
      }
      auto const count =
        std::min(static_cast<std::ptrdiff_t>(buffer.size()), std::ptrdiff_t{2});
      auto const available =
        static_cast<std::ptrdiff_t>(_bytes.size()) - _offset;
      auto const actual = std::min(count, available);
      for (auto i = std::ptrdiff_t{0}; i < actual; ++i)
      {
        buffer[i] = _bytes[_offset + i];
      }
      _offset += actual;
      return actual;
    }

  private:
    std::array<uint8_t, 4> _bytes{'x', 'y', 'z', '!'};
    std::ptrdiff_t _offset{};
  };

  auto binary = Chunked_binary_input_stream{};
  REQUIRE(binary.read_byte() == 'x');
  REQUIRE(binary.read_byte() == 'y');
  REQUIRE(binary.read_byte() == 'z');
  REQUIRE(binary.read_byte() == '!');
  REQUIRE(!binary.read_byte());
}
