#include <algorithm>
#include <array>
#include <cstddef>
#include <cstdint>
#include <vector>

#include <catch2/catch_test_macros.hpp>

#include "streams/binary_output_stream.h"
#include "streams/binary_output_stream_writer.h"

TEST_CASE("Binary_output_stream_writer")
{
  class Recording_binary_output_stream: public benson::Binary_output_stream
  {
  public:
    explicit Recording_binary_output_stream(std::ptrdiff_t max_write_size = 2)
        : _max_write_size{max_write_size}
    {
    }

    std::ptrdiff_t write_bytes(std::span<uint8_t const> buffer) override
    {
      if (buffer.empty())
      {
        return 0;
      }
      auto const count =
        std::min(static_cast<std::ptrdiff_t>(buffer.size()), _max_write_size);
      _bytes.insert(_bytes.end(), buffer.begin(), buffer.begin() + count);
      _write_sizes.push_back(count);
      return count;
    }

    [[nodiscard]] auto bytes() const -> std::vector<uint8_t> const &
    {
      return _bytes;
    }

    [[nodiscard]] auto write_sizes() const
      -> std::vector<std::ptrdiff_t> const &
    {
      return _write_sizes;
    }

  private:
    std::vector<uint8_t> _bytes{};
    std::vector<std::ptrdiff_t> _write_sizes{};
    std::ptrdiff_t _max_write_size;
  };

  SECTION("sequential byte writes preserve order across buffer boundaries")
  {
    auto binary = Recording_binary_output_stream{512};
    auto writer = benson::Binary_output_stream_writer{&binary};

    for (auto i = 0; i < 5000; ++i)
    {
      writer.write(static_cast<std::byte>(i % 251));
    }
    writer.flush();

    auto expected = std::vector<uint8_t>{};
    expected.reserve(5000);
    for (auto i = 0; i < 5000; ++i)
    {
      expected.push_back(static_cast<uint8_t>(i % 251));
    }
    CHECK(binary.bytes() == expected);
  }

  SECTION("span writes preserve order and exact byte values")
  {
    auto binary = Recording_binary_output_stream{4096};
    auto writer = benson::Binary_output_stream_writer{&binary};
    auto const bytes = std::array<std::byte, 4>{
      std::byte{0x00},
      std::byte{0x7f},
      std::byte{0x80},
      std::byte{0xff}
    };

    writer.write(std::span<std::byte const>{bytes});
    writer.flush();

    CHECK(binary.bytes() == std::vector<uint8_t>{0x00, 0x7f, 0x80, 0xff});
  }

  SECTION("mixed byte and span writes compose correctly")
  {
    auto binary = Recording_binary_output_stream{4096};
    auto writer = benson::Binary_output_stream_writer{&binary};
    auto const middle =
      std::array<std::byte, 2>{std::byte{'b'}, std::byte{'c'}};

    writer.write(std::byte{'a'});
    writer.write(std::span<std::byte const>{middle});
    writer.write(std::byte{'d'});
    writer.flush();

    CHECK(binary.bytes() == std::vector<uint8_t>{'a', 'b', 'c', 'd'});
  }

  SECTION("short writes flush all buffered and direct-written bytes")
  {
    auto binary = Recording_binary_output_stream{3};
    auto writer = benson::Binary_output_stream_writer{&binary};
    auto const bytes = std::array<std::byte, 7>{
      std::byte{'a'},
      std::byte{'b'},
      std::byte{'c'},
      std::byte{'d'},
      std::byte{'e'},
      std::byte{'f'},
      std::byte{'g'}
    };

    writer.write(std::span<std::byte const>{bytes});
    writer.flush();

    CHECK(
      binary.bytes() == std::vector<uint8_t>{'a', 'b', 'c', 'd', 'e', 'f', 'g'}
    );
    CHECK(binary.write_sizes() == std::vector<std::ptrdiff_t>{3, 3, 1});
  }

  SECTION("large span writes bypass staging after flushing pending bytes")
  {
    auto binary = Recording_binary_output_stream{1024};
    auto writer = benson::Binary_output_stream_writer{&binary};
    auto const bytes = [&]()
    {
      auto result = std::array<std::byte, 5000>{};
      for (auto i = std::size_t{}; i < result.size(); ++i)
      {
        result[i] = static_cast<std::byte>(i % 251);
      }
      return result;
    }();

    writer.write(std::byte{'x'});
    writer.write(std::span<std::byte const>{bytes});

    REQUIRE(binary.bytes().size() == 5001);
    CHECK(binary.bytes().front() == static_cast<uint8_t>('x'));
    CHECK(
      std::equal(
        bytes.begin(),
        bytes.end(),
        binary.bytes().begin() + 1,
        [](std::byte expected, uint8_t actual)
        {
          return std::to_integer<uint8_t>(expected) == actual;
        }
      )
    );
  }

  SECTION("flush drains pending bytes")
  {
    auto binary = Recording_binary_output_stream{4096};
    auto writer = benson::Binary_output_stream_writer{&binary};

    writer.write(std::byte{'a'});
    writer.write(std::byte{'b'});
    writer.flush();

    CHECK(binary.bytes() == std::vector<uint8_t>{'a', 'b'});
  }

  SECTION("destruction without flush does not write pending bytes")
  {
    auto binary = Recording_binary_output_stream{4096};

    {
      auto writer = benson::Binary_output_stream_writer{&binary};
      writer.write(std::byte{'a'});
    }

    CHECK(binary.bytes().empty());
  }

  SECTION("empty flush and empty span write are no-ops")
  {
    auto binary = Recording_binary_output_stream{4096};
    auto writer = benson::Binary_output_stream_writer{&binary};
    auto const empty = std::span<std::byte const>{};

    writer.write(empty);
    writer.flush();

    CHECK(binary.bytes().empty());
    CHECK(binary.write_sizes().empty());
  }
}
