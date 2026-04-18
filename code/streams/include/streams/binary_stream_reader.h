#ifndef BASEDSTREAMS_BINARY_STREAM_READER_H
#define BASEDSTREAMS_BINARY_STREAM_READER_H

#include <array>
#include <cstddef>
#include <cstdint>
#include <memory>
#include <optional>

#include "streams/binary_stream.h"

namespace benson
{

  /// Buffers a Binary_stream behind a 4 KiB heap-allocated scratch buffer and
  /// exposes byte-level reads. Refills the buffer via the bulk read_bytes API
  /// so callers amortize virtual dispatch across many bytes.
  class Binary_stream_reader
  {
  public:
    explicit Binary_stream_reader(Binary_stream *stream)
        : _stream{stream},
          _buffer{std::make_unique<std::array<uint8_t, 4096>>()}
    {
    }

    /// Returns the next byte, or std::nullopt at EOF.
    std::optional<uint8_t> read()
    {
      if (_pos == _end)
      {
        _pos = 0;
        _end = _stream->read_bytes(*_buffer);
        if (_end == 0)
        {
          return std::nullopt;
        }
      }
      return (*_buffer)[_pos++];
    }

  private:
    Binary_stream *_stream;
    std::unique_ptr<std::array<uint8_t, 4096>> _buffer;
    std::ptrdiff_t _pos{};
    std::ptrdiff_t _end{};
  };

} // namespace benson

#endif // BASEDSTREAMS_BINARY_STREAM_READER_H
