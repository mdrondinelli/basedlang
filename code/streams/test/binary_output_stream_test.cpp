#include <algorithm>
#include <array>
#include <vector>

#include <catch2/catch_test_macros.hpp>

#include "streams/binary_output_stream.h"

TEST_CASE("Binary_output_stream - write_bytes")
{
  class Recording_binary_output_stream: public benson::Binary_output_stream
  {
  public:
    std::ptrdiff_t write_bytes(std::span<uint8_t const> buffer) override
    {
      if (buffer.empty())
      {
        return 0;
      }
      auto const count =
        std::min(static_cast<std::ptrdiff_t>(buffer.size()), _max_write_size);
      _bytes.insert(_bytes.end(), buffer.begin(), buffer.begin() + count);
      return count;
    }

    [[nodiscard]] auto bytes() const -> std::vector<uint8_t> const &
    {
      return _bytes;
    }

  private:
    std::vector<uint8_t> _bytes{};
    std::ptrdiff_t _max_write_size{2};
  };

  SECTION("empty buffer is no-op")
  {
    auto binary = Recording_binary_output_stream{};
    auto empty = std::span<uint8_t const>{};
    REQUIRE(binary.write_bytes(empty) == 0);
    CHECK(binary.bytes().empty());
  }

  SECTION("non-empty buffer may short-write with positive count")
  {
    auto binary = Recording_binary_output_stream{};
    auto const buffer = std::array<uint8_t, 4>{'a', 'b', 'c', 'd'};
    REQUIRE(binary.write_bytes(buffer) == 2);
    CHECK(binary.bytes() == std::vector<uint8_t>{'a', 'b'});
  }

  SECTION("repeated writes preserve byte order")
  {
    auto binary = Recording_binary_output_stream{};
    auto const first = std::array<uint8_t, 3>{'a', 'b', 'c'};
    auto const second = std::array<uint8_t, 2>{'d', 'e'};
    auto const first_span = std::span<uint8_t const>{first};
    REQUIRE(binary.write_bytes(first) == 2);
    REQUIRE(binary.write_bytes(first_span.subspan(2)) == 1);
    REQUIRE(binary.write_bytes(second) == 2);
    CHECK(binary.bytes() == std::vector<uint8_t>{'a', 'b', 'c', 'd', 'e'});
  }
}
