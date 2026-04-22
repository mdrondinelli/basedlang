#ifndef BASEDSTREAMS_CHAR_INPUT_STREAM_PEEKER_H
#define BASEDSTREAMS_CHAR_INPUT_STREAM_PEEKER_H

#include <bit>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <memory>
#include <optional>

#include "streams/char_input_stream.h"

namespace benson
{

  /// Wraps a Char_input_stream with finite lookahead, exposing peek/read
  /// operations backed by a ring buffer.
  class Char_input_stream_peeker
  {
  public:
    Char_input_stream_peeker(
      Char_input_stream *stream,
      std::ptrdiff_t max_lookahead
    )
        : _stream{stream},
          _buffer(
            [&]()
            {
              assert(max_lookahead >= 0);
              return std::make_unique<uint32_t[]>(
                std::bit_ceil(static_cast<std::size_t>(max_lookahead + 1))
              );
            }()
          ),
          _capacity{std::bit_ceil(static_cast<std::size_t>(max_lookahead + 1))},
          _max_lookahead{max_lookahead}
    {
    }

    std::optional<uint32_t> peek(std::ptrdiff_t offset = 0)
    {
      assert(offset >= 0);
      assert(offset <= _max_lookahead);
      auto const needed = static_cast<std::size_t>(offset + 1);
      while (_size < needed)
      {
        auto const next = _stream->read_character();
        if (!next)
        {
          return std::nullopt;
        }
        _buffer[(_head + _size) & (_capacity - 1)] = *next;
        ++_size;
      }
      return _buffer
        [(_head + static_cast<std::size_t>(offset)) & (_capacity - 1)];
    }

    std::optional<uint32_t> read()
    {
      if (_size == 0)
      {
        return _stream->read_character();
      }
      auto const result = _buffer[_head];
      _head = (_head + 1) & (_capacity - 1);
      --_size;
      return result;
    }

  private:
    Char_input_stream *_stream;
    std::unique_ptr<uint32_t[]> _buffer;
    std::size_t _capacity;
    std::size_t _head{};
    std::size_t _size{};
    std::ptrdiff_t _max_lookahead;
  };

} // namespace benson

#endif // BASEDSTREAMS_CHAR_INPUT_STREAM_PEEKER_H
