#include <cstddef>
#include <vector>

#include <catch2/catch_test_macros.hpp>

#include "streams/binary_output_stream.h"
#include "streams/binary_output_stream_writer.h"

TEST_CASE("Binary_output_stream_writer")
{
  class Recording_binary_output_stream: public benson::Binary_output_stream
  {
  public:
    void write_bytes(std::span<std::byte const> buffer) override
    {
      _bytes.insert(_bytes.end(), buffer.begin(), buffer.end());
      _write_sizes.push_back(static_cast<std::ptrdiff_t>(buffer.size()));
    }

    [[nodiscard]] auto bytes() const -> std::vector<std::byte> const &
    {
      return _bytes;
    }

    [[nodiscard]] auto write_sizes() const
      -> std::vector<std::ptrdiff_t> const &
    {
      return _write_sizes;
    }

  private:
    std::vector<std::byte> _bytes{};
    std::vector<std::ptrdiff_t> _write_sizes{};
  };

  SECTION("sequential byte writes preserve order across buffer boundaries")
  {
    auto binary = Recording_binary_output_stream{};
    auto writer = benson::Binary_output_stream_writer{&binary};

    for (auto i = 0; i < 5000; ++i)
    {
      writer.write(static_cast<std::byte>(i % 251));
    }
    writer.flush();

    auto expected = std::vector<std::byte>{};
    expected.reserve(5000);
    for (auto i = 0; i < 5000; ++i)
    {
      expected.push_back(static_cast<std::byte>(i % 251));
    }
    CHECK(binary.bytes() == expected);
  }

  SECTION("flush calls write_bytes exactly once")
  {
    auto binary = Recording_binary_output_stream{};
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
      binary.bytes() == std::vector<std::byte>{
                          std::byte{'a'},
                          std::byte{'b'},
                          std::byte{'c'},
                          std::byte{'d'},
                          std::byte{'e'},
                          std::byte{'f'},
                          std::byte{'g'},
                        }
    );
    CHECK(binary.write_sizes() == std::vector<std::ptrdiff_t>{7});
  }

  SECTION("flush drains pending bytes")
  {
    auto binary = Recording_binary_output_stream{};
    auto writer = benson::Binary_output_stream_writer{&binary};

    writer.write(std::byte{'a'});
    writer.write(std::byte{'b'});
    writer.flush();

    CHECK(
      binary.bytes() == std::vector<std::byte>{std::byte{'a'}, std::byte{'b'}}
    );
  }

  SECTION("destruction without flush does not write pending bytes")
  {
    auto binary = Recording_binary_output_stream{};

    {
      auto writer = benson::Binary_output_stream_writer{&binary};
      writer.write(std::byte{'a'});
    }

    CHECK(binary.bytes().empty());
  }

  SECTION("empty flush is no-op")
  {
    auto binary = Recording_binary_output_stream{};
    auto writer = benson::Binary_output_stream_writer{&binary};
    writer.flush();

    CHECK(binary.bytes().empty());
    CHECK(binary.write_sizes().empty());
  }
}
