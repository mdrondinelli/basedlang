#ifndef BASEDSTREAMS_LOOKAHEAD_CHAR_STREAM_READER_H
#define BASEDSTREAMS_LOOKAHEAD_CHAR_STREAM_READER_H

#include <cassert>
#include <cstddef>
#include <cstdint>
#include <optional>
#include <vector>

#include "streams/char_stream.h"
#include "streams/char_stream_reader.h"

namespace benson
{

  /// Wraps a Char_stream with finite lookahead, exposing peek/read operations
  /// backed by a ring buffer.
  class Lookahead_char_stream_reader
  {
  public:
    Lookahead_char_stream_reader(
      Char_stream *stream,
      std::ptrdiff_t max_lookahead
    )
        : _reader{stream},
          _buffer([&]()
          {
            assert(max_lookahead >= 0);
            return std::vector<uint32_t>(
              static_cast<std::size_t>(max_lookahead + 1)
            );
          }()),
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
        auto const next = _reader.read();
        if (!next)
        {
          return std::nullopt;
        }
        _buffer[(_head + _size) % _buffer.size()] = *next;
        ++_size;
      }
      return _buffer
        [(_head + static_cast<std::size_t>(offset)) % _buffer.size()];
    }

    std::optional<uint32_t> read()
    {
      if (_size == 0)
      {
        return _reader.read();
      }
      auto const result = _buffer[_head];
      _head = (_head + 1) % _buffer.size();
      --_size;
      return result;
    }

  private:
    Char_stream_reader _reader;
    std::vector<uint32_t> _buffer;
    std::size_t _head{};
    std::size_t _size{};
    std::ptrdiff_t _max_lookahead;
  };

} // namespace benson

#endif // BASEDSTREAMS_LOOKAHEAD_CHAR_STREAM_READER_H
