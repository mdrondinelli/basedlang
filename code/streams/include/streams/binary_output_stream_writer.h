#ifndef BASEDSTREAMS_BINARY_OUTPUT_STREAM_WRITER_H
#define BASEDSTREAMS_BINARY_OUTPUT_STREAM_WRITER_H

#include <array>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <memory>
#include <span>

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
          _buffer{std::make_unique<std::array<uint8_t, 4096>>()}
    {
    }

    void write(std::byte byte)
    {
      if (_size == static_cast<std::ptrdiff_t>(_buffer->size()))
      {
        flush();
      }
      (*_buffer)[_size++] = std::to_integer<uint8_t>(byte);
    }

    void write(std::span<std::byte const> bytes)
    {
      if (bytes.empty())
      {
        return;
      }
      auto const capacity = static_cast<std::ptrdiff_t>(_buffer->size());
      auto const count = static_cast<std::ptrdiff_t>(bytes.size());
      if (count <= capacity - _size)
      {
        std::memcpy(_buffer->data() + _size, bytes.data(), bytes.size());
        _size += count;
        return;
      }
      if (_size != 0)
      {
        flush();
      }
      if (count >= capacity)
      {
        write_direct(bytes);
        return;
      }
      std::memcpy(_buffer->data(), bytes.data(), bytes.size());
      _size = count;
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
    static auto as_uint8_span(std::span<std::byte const> bytes)
      -> std::span<uint8_t const>
    {
      auto const *data = reinterpret_cast<uint8_t const *>(bytes.data());
      return std::span{data, bytes.size()};
    }

    void write_direct(std::span<std::byte const> bytes)
    {
      auto remaining = as_uint8_span(bytes);
      while (!remaining.empty())
      {
        auto const wrote = _stream->write_bytes(remaining);
        remaining = remaining.subspan(wrote);
      }
    }

    Binary_output_stream *_stream;
    std::unique_ptr<std::array<uint8_t, 4096>> _buffer;
    std::ptrdiff_t _size{};
  };

} // namespace benson

#endif // BASEDSTREAMS_BINARY_OUTPUT_STREAM_WRITER_H
