#ifndef BASEDSTREAMS_BINARY_OUTPUT_STREAM_H
#define BASEDSTREAMS_BINARY_OUTPUT_STREAM_H

#include <cstddef>
#include <span>

namespace benson
{

  /// Abstract sink for raw bytes.
  class Binary_output_stream
  {
  public:
    virtual ~Binary_output_stream() = default;

    /// Writes up to buffer.size() bytes from buffer and returns number of bytes
    /// written. Implementations may return short writes, but when buffer is
    /// non-empty a short write must be strictly positive; a return of 0 then
    /// requires buffer to be empty. When buffer is empty, call is a no-op and
    /// returns 0. If an implementation throws after consuming bytes from
    /// buffer, stream state is undefined.
    virtual std::ptrdiff_t write_bytes(std::span<std::byte const> buffer) = 0;
  };

} // namespace benson

#endif // BASEDSTREAMS_BINARY_OUTPUT_STREAM_H
