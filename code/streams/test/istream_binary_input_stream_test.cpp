#include <algorithm>
#include <array>
#include <cstddef>
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
    auto buffer = std::array<std::byte, 4>{};
    REQUIRE(binary.read_bytes(buffer) == 0);
  }
  SECTION("empty buffer is no-op")
  {
    auto ss = std::istringstream{"hello"};
    auto binary = benson::Istream_binary_input_stream{&ss};
    auto empty = std::span<std::byte>{};
    auto buffer = std::array<std::byte, 5>{};
    REQUIRE(binary.read_bytes(empty) == 0);
    REQUIRE(binary.read_bytes(buffer) == 5);
    CHECK(buffer[0] == std::byte{'h'});
    CHECK(buffer[1] == std::byte{'e'});
    CHECK(buffer[2] == std::byte{'l'});
    CHECK(buffer[3] == std::byte{'l'});
    CHECK(buffer[4] == std::byte{'o'});
  }
  SECTION("fills full buffer when enough bytes exist")
  {
    auto ss = std::istringstream{"hello"};
    auto binary = benson::Istream_binary_input_stream{&ss};
    auto buffer = std::array<std::byte, 4>{};
    REQUIRE(binary.read_bytes(buffer) == 4);
    CHECK(buffer[0] == std::byte{'h'});
    CHECK(buffer[1] == std::byte{'e'});
    CHECK(buffer[2] == std::byte{'l'});
    CHECK(buffer[3] == std::byte{'l'});
  }
  SECTION("short final read returns remaining bytes")
  {
    auto ss = std::istringstream{"hello"};
    auto binary = benson::Istream_binary_input_stream{&ss};
    auto buffer = std::array<std::byte, 8>{};
    REQUIRE(binary.read_bytes(buffer) == 5);
    CHECK(buffer[0] == std::byte{'h'});
    CHECK(buffer[1] == std::byte{'e'});
    CHECK(buffer[2] == std::byte{'l'});
    CHECK(buffer[3] == std::byte{'l'});
    CHECK(buffer[4] == std::byte{'o'});
    REQUIRE(binary.read_bytes(buffer) == 0);
  }
  SECTION("repeated reads reconstruct source")
  {
    auto ss = std::istringstream{"abcdefg"};
    auto binary = benson::Istream_binary_input_stream{&ss};
    auto buffer = std::array<std::byte, 3>{};
    auto bytes = std::vector<std::byte>{};
    for (;;)
    {
      auto const count = binary.read_bytes(buffer);
      if (count == 0)
      {
        break;
      }
      bytes.insert(bytes.end(), buffer.begin(), buffer.begin() + count);
    }
    REQUIRE(
      bytes == std::vector<std::byte>{
        std::byte{'a'},
        std::byte{'b'},
        std::byte{'c'},
        std::byte{'d'},
        std::byte{'e'},
        std::byte{'f'},
        std::byte{'g'},
      }
    );
  }
}

TEST_CASE("Binary_input_stream - read_byte compatibility helper")
{
  class Chunked_binary_input_stream: public benson::Binary_input_stream
  {
  public:
    std::ptrdiff_t read_bytes(std::span<std::byte> buffer) override
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
    std::array<std::byte, 4> _bytes{
      std::byte{'x'},
      std::byte{'y'},
      std::byte{'z'},
      std::byte{'!'},
    };
    std::ptrdiff_t _offset{};
  };

  auto binary = Chunked_binary_input_stream{};
  REQUIRE(binary.read_byte() == std::byte{'x'});
  REQUIRE(binary.read_byte() == std::byte{'y'});
  REQUIRE(binary.read_byte() == std::byte{'z'});
  REQUIRE(binary.read_byte() == std::byte{'!'});
  REQUIRE(!binary.read_byte());
}
