#ifndef BASEDSTREAMS_CHAR_STREAM_READER_H
#define BASEDSTREAMS_CHAR_STREAM_READER_H

#include <cassert>
#include <cstddef>
#include <cstdint>
#include <optional>
#include <vector>

#include "streams/char_stream.h"

namespace benson
{

  /// Wraps a Char_stream with arbitrary lookahead, exposing peek/read
  /// operations.
  class Char_stream_reader
  {
  public:
    explicit Char_stream_reader(Char_stream *stream) noexcept
        : _stream{stream}
    {
    }

    std::optional<uint32_t> peek(std::ptrdiff_t offset = 0)
    {
      assert(offset >= 0);
      auto const index = static_cast<std::size_t>(offset);
      buffer_to(index + 1);
      if (index < _buffer.size())
      {
        return _buffer[index];
      }
      return std::nullopt;
    }

    std::optional<uint32_t> read()
    {
      buffer_to(1);
      if (_buffer.empty())
      {
        return std::nullopt;
      }
      auto const result = _buffer.front();
      _buffer.erase(_buffer.begin());
      return result;
    }

  private:
    void buffer_to(std::size_t count)
    {
      while (_buffer.size() < count)
      {
        auto const c = _stream->read_character();
        if (!c)
        {
          return;
        }
        _buffer.push_back(*c);
      }
    }

    Char_stream *_stream;
    std::vector<uint32_t> _buffer;
  };

} // namespace benson

#endif // BASEDSTREAMS_CHAR_STREAM_READER_H
