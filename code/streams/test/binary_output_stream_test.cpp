#include <algorithm>
#include <array>
#include <cstddef>
#include <vector>

#include <catch2/catch_test_macros.hpp>

#include "streams/binary_output_stream.h"

TEST_CASE("Binary_output_stream - write_bytes")
{
  class Recording_binary_output_stream: public benson::Binary_output_stream
  {
  public:
    std::ptrdiff_t write_bytes(std::span<std::byte const> buffer) override
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

    [[nodiscard]] auto bytes() const -> std::vector<std::byte> const &
    {
      return _bytes;
    }

  private:
    std::vector<std::byte> _bytes{};
    std::ptrdiff_t _max_write_size{2};
  };

  SECTION("empty buffer is no-op")
  {
    auto binary = Recording_binary_output_stream{};
    auto empty = std::span<std::byte const>{};
    REQUIRE(binary.write_bytes(empty) == 0);
    CHECK(binary.bytes().empty());
  }

  SECTION("non-empty buffer may short-write with positive count")
  {
    auto binary = Recording_binary_output_stream{};
    auto const buffer = std::array{
      std::byte{'a'},
      std::byte{'b'},
      std::byte{'c'},
      std::byte{'d'}
    };
    REQUIRE(binary.write_bytes(buffer) == 2);
    CHECK(
      binary.bytes() == std::vector<std::byte>{std::byte{'a'}, std::byte{'b'}}
    );
  }

  SECTION("repeated writes preserve byte order")
  {
    auto binary = Recording_binary_output_stream{};
    auto const first =
      std::array{std::byte{'a'}, std::byte{'b'}, std::byte{'c'}};
    auto const second = std::array{std::byte{'d'}, std::byte{'e'}};
    auto const first_span = std::span<std::byte const>{first};
    REQUIRE(binary.write_bytes(first) == 2);
    REQUIRE(binary.write_bytes(first_span.subspan(2)) == 1);
    REQUIRE(binary.write_bytes(second) == 2);
    CHECK(
      binary.bytes() == std::vector<std::byte>{
                          std::byte{'a'},
                          std::byte{'b'},
                          std::byte{'c'},
                          std::byte{'d'},
                          std::byte{'e'},
                        }
    );
  }
}
