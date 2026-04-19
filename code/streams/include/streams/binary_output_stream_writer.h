#ifndef BASEDSTREAMS_BINARY_OUTPUT_STREAM_WRITER_H
#define BASEDSTREAMS_BINARY_OUTPUT_STREAM_WRITER_H

#include <array>
#include <cstddef>
#include <memory>

#include "streams/binary_output_stream.h"

namespace benson
{

  /// Buffers a Binary_output_stream behind a 4 KiB heap-allocated scratch
  /// buffer and exposes byte-level writes. Flushes the buffer via the bulk
  /// write_bytes API so callers amortize virtual dispatch across many bytes.
  class Binary_output_stream_writer
  {
  public:
    explicit Binary_output_stream_writer(Binary_output_stream *stream)
        : _stream{stream},
          _buffer{std::make_unique<std::array<std::byte, 4096>>()}
    {
    }

    void write(std::byte byte)
    {
      if (_size == static_cast<std::ptrdiff_t>(_buffer->size()))
      {
        flush();
      }
      (*_buffer)[_size++] = byte;
    }

    void flush()
    {
      auto pos = std::ptrdiff_t{};
      while (pos < _size)
      {
        auto const remaining = _size - pos;
        auto const wrote = _stream->write_bytes(
          std::span{_buffer->data() + pos, static_cast<std::size_t>(remaining)}
        );
        pos += wrote;
      }
      _size = 0;
    }

  private:
    Binary_output_stream *_stream;
    std::unique_ptr<std::array<std::byte, 4096>> _buffer;
    std::ptrdiff_t _size{};
  };

} // namespace benson

#endif // BASEDSTREAMS_BINARY_OUTPUT_STREAM_WRITER_H
