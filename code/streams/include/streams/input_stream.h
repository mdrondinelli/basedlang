#ifndef BASEDSTREAMS_INPUT_STREAM_H
#define BASEDSTREAMS_INPUT_STREAM_H

#include <cstddef>
#include <span>

namespace benson
{

  /// Abstract source of raw bytes. Implement this to feed binary data into a
  /// Utf8_char_input_stream.
  class Input_stream
  {
  public:
    virtual ~Input_stream() = default;

    /// Reads up to buffer.size() bytes into buffer and returns the number of
    /// bytes read. Implementations may return short reads, but when buffer is
    /// non-empty a short read must be strictly positive; a return of 0 then
    /// means EOF. When buffer is empty, the call is a no-op and returns 0. If
    /// an implementation throws after writing bytes into buffer, those bytes
    /// are lost and the stream position is undefined.
    virtual std::ptrdiff_t read_bytes(std::span<std::byte> buffer) = 0;
  };

} // namespace benson

#endif // BASEDSTREAMS_INPUT_STREAM_H
