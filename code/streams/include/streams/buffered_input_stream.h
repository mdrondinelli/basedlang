#ifndef BASEDSTREAMS_BUFFERED_INPUT_STREAM_H
#define BASEDSTREAMS_BUFFERED_INPUT_STREAM_H

#include <algorithm>
#include <array>
#include <concepts>
#include <cstddef>
#include <memory>
#include <utility>

#include "streams/input_stream.h"

namespace benson
{

  /// Buffers a concrete Input_stream behind a 4 KiB heap-allocated
  /// scratch buffer while still exposing the Input_stream interface.
  /// Preserves the Input_stream contract: callers may still observe
  /// short positive reads and EOF as 0, but repeated small reads can be served
  /// from the internal buffer without re-entering the wrapped stream. Large
  /// reads bypass the scratch buffer once it is empty.
  template <std::derived_from<Input_stream> T>
  class Buffered_input_stream: public Input_stream
  {
  public:
    template <typename... Args>
    explicit Buffered_input_stream(Args &&...args)
        : _stream{std::forward<Args>(args)...},
          _buffer{std::make_unique<std::array<std::byte, 4096>>()}
    {
    }

    std::ptrdiff_t read_bytes(std::span<std::byte> buffer) override
    {
      if (buffer.empty())
      {
        return 0;
      }

      auto total = std::ptrdiff_t{};
      auto const requested = static_cast<std::ptrdiff_t>(buffer.size());
      for (;;)
      {
        if (_pos == _end)
        {
          // Large direct reads do not benefit from copying through the
          // intermediate scratch buffer.
          if (requested - total >= static_cast<std::ptrdiff_t>(_buffer->size()))
          {
            return total + _stream.read_bytes(buffer.subspan(total));
          }

          _pos = 0;
          _end = _stream.read_bytes(*_buffer);
          if (_end == 0)
          {
            return total;
          }
        }

        auto const available = _end - _pos;
        auto const remaining = requested - total;
        if (available < remaining)
        {
          std::copy_n(
            _buffer->data() + _pos,
            static_cast<std::size_t>(available),
            buffer.data() + total
          );
          _pos = _end;
          total += available;
        }
        else
        {
          std::copy_n(
            _buffer->data() + _pos,
            static_cast<std::size_t>(remaining),
            buffer.data() + total
          );
          _pos += remaining;
          return requested;
        }
      }
    }

  private:
    T _stream;
    std::unique_ptr<std::array<std::byte, 4096>> _buffer;
    std::ptrdiff_t _pos{};
    std::ptrdiff_t _end{};
  };

} // namespace benson

#endif // BASEDSTREAMS_BUFFERED_INPUT_STREAM_H
