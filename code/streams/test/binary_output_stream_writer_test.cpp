#include <algorithm>
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
    explicit Recording_binary_output_stream(std::ptrdiff_t max_write_size)
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

  SECTION("flush retries on stream short-writes")
  {
    auto binary = Recording_binary_output_stream{3};
    auto writer = benson::Binary_output_stream_writer{&binary};

    writer.write(std::byte{'a'});
    writer.write(std::byte{'b'});
    writer.write(std::byte{'c'});
    writer.write(std::byte{'d'});
    writer.write(std::byte{'e'});
    writer.write(std::byte{'f'});
    writer.write(std::byte{'g'});
    writer.flush();

    CHECK(
      binary.bytes() == std::vector<uint8_t>{'a', 'b', 'c', 'd', 'e', 'f', 'g'}
    );
    CHECK(binary.write_sizes() == std::vector<std::ptrdiff_t>{3, 3, 1});
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

  SECTION("empty flush is no-op")
  {
    auto binary = Recording_binary_output_stream{4096};
    auto writer = benson::Binary_output_stream_writer{&binary};
    writer.flush();

    CHECK(binary.bytes().empty());
    CHECK(binary.write_sizes().empty());
  }
}
