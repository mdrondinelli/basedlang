#ifndef BASEDSTREAMS_OUTPUT_STREAM_H
#define BASEDSTREAMS_OUTPUT_STREAM_H

#include <cstddef>
#include <span>

namespace benson
{

  /// Abstract sink for raw bytes.
  class Output_stream
  {
  public:
    virtual ~Output_stream() = default;

    /// Writes all bytes in buffer to the stream. When buffer is empty, call is
    /// a no-op. If an implementation throws after consuming some bytes from
    /// buffer, stream state is undefined.
    virtual void write_bytes(std::span<std::byte const> buffer) = 0;

    /// Flushes any buffered writes to the underlying sink. The default
    /// implementation is a no-op for unbuffered sinks.
    virtual void flush()
    {
    }
  };

} // namespace benson

#endif // BASEDSTREAMS_OUTPUT_STREAM_H
