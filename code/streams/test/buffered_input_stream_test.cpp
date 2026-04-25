#include <algorithm>
#include <array>
#include <cstddef>
#include <vector>

#include <catch2/catch_test_macros.hpp>

#include "streams/buffered_input_stream.h"
#include "streams/input_stream.h"

namespace
{

  class Chunked_input_stream: public benson::Input_stream
  {
  public:
    Chunked_input_stream(
      std::vector<std::byte> bytes,
      std::ptrdiff_t chunk_size,
      std::ptrdiff_t *read_calls
    )
        : _bytes{std::move(bytes)},
          _chunk_size{chunk_size},
          _read_calls{read_calls}
    {
    }

    std::ptrdiff_t read_bytes(std::span<std::byte> buffer) override
    {
      ++(*_read_calls);
      if (buffer.empty() ||
          _offset == static_cast<std::ptrdiff_t>(_bytes.size()))
      {
        return 0;
      }

      auto const available =
        static_cast<std::ptrdiff_t>(_bytes.size()) - _offset;
      auto const count = std::min(
        static_cast<std::ptrdiff_t>(buffer.size()),
        std::min(_chunk_size, available)
      );
      std::copy_n(
        _bytes.data() + _offset,
        static_cast<std::size_t>(count),
        buffer.data()
      );
      _offset += count;
      return count;
    }

  private:
    std::vector<std::byte> _bytes;
    std::ptrdiff_t _chunk_size;
    std::ptrdiff_t *_read_calls;
    std::ptrdiff_t _offset{};
  };

} // namespace

TEST_CASE("Buffered_input_stream amortizes repeated tiny reads")
{
  auto read_calls = std::ptrdiff_t{};
  auto buffered = benson::Buffered_input_stream<Chunked_input_stream>{
    std::vector<std::byte>{
      std::byte{'h'},
      std::byte{'e'},
      std::byte{'l'},
      std::byte{'l'},
      std::byte{'o'},
    },
    2,
    &read_calls,
  };

  auto byte = std::array<std::byte, 1>{};
  auto bytes = std::vector<std::byte>{};
  for (;;)
  {
    auto const count = buffered.read_bytes(byte);
    if (count == 0)
    {
      break;
    }
    bytes.push_back(byte[0]);
  }

  CHECK(
    bytes == std::vector<std::byte>{
               std::byte{'h'},
               std::byte{'e'},
               std::byte{'l'},
               std::byte{'l'},
               std::byte{'o'},
             }
  );
  CHECK(read_calls == 4);
}

TEST_CASE("Buffered_input_stream returns buffered bytes before refilling")
{
  auto read_calls = std::ptrdiff_t{};
  auto buffered = benson::Buffered_input_stream<Chunked_input_stream>{
    std::vector<std::byte>{
      std::byte{'a'},
      std::byte{'b'},
      std::byte{'c'},
      std::byte{'d'},
    },
    4,
    &read_calls,
  };

  auto first = std::array<std::byte, 3>{};
  REQUIRE(buffered.read_bytes(first) == 3);
  CHECK(
    std::vector<std::byte>{first.begin(), first.end()} ==
    std::vector<std::byte>{std::byte{'a'}, std::byte{'b'}, std::byte{'c'}}
  );

  auto second = std::array<std::byte, 1>{};
  REQUIRE(buffered.read_bytes(second) == 1);
  CHECK(second[0] == std::byte{'d'});
  CHECK(read_calls == 1);
}
