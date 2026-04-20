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
    void write_bytes(std::span<std::byte const> buffer) override
    {
      _bytes.insert(_bytes.end(), buffer.begin(), buffer.end());
    }

    [[nodiscard]] auto bytes() const -> std::vector<std::byte> const &
    {
      return _bytes;
    }

  private:
    std::vector<std::byte> _bytes{};
  };

  SECTION("empty buffer is no-op")
  {
    auto binary = Recording_binary_output_stream{};
    binary.write_bytes(std::span<std::byte const>{});
    CHECK(binary.bytes().empty());
  }

  SECTION("non-empty buffer writes all bytes")
  {
    auto binary = Recording_binary_output_stream{};
    auto const buffer = std::array{
      std::byte{'a'},
      std::byte{'b'},
      std::byte{'c'},
      std::byte{'d'}
    };
    binary.write_bytes(buffer);
    CHECK(
      binary.bytes() == std::vector<std::byte>{
                          std::byte{'a'},
                          std::byte{'b'},
                          std::byte{'c'},
                          std::byte{'d'},
                        }
    );
  }

  SECTION("repeated writes preserve byte order")
  {
    auto binary = Recording_binary_output_stream{};
    auto const first =
      std::array{std::byte{'a'}, std::byte{'b'}, std::byte{'c'}};
    auto const second = std::array{std::byte{'d'}, std::byte{'e'}};
    binary.write_bytes(first);
    binary.write_bytes(second);
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
