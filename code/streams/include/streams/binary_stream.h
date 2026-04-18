#ifndef BASEDSTREAMS_BINARY_STREAM_H
#define BASEDSTREAMS_BINARY_STREAM_H

#include <cstddef>
#include <cstdint>
#include <optional>
#include <span>

namespace benson
{

  /// Abstract source of raw bytes. Implement this to feed binary data into a
  /// Utf8_char_stream.
  class Binary_stream
  {
  public:
    virtual ~Binary_stream() = default;

    /// Reads up to buffer.size() bytes into buffer and returns the number of
    /// bytes read. Returns 0 at EOF. Implementations may return short reads.
    virtual std::size_t read_bytes(std::span<uint8_t> buffer) = 0;

    /// Returns the next byte, or std::nullopt at EOF.
    std::optional<uint8_t> read_byte()
    {
      auto buffer = uint8_t{};
      if (read_bytes(std::span{&buffer, std::size_t{1}}) == 0)
      {
        return std::nullopt;
      }
      return buffer;
    }
  };

} // namespace benson

#endif // BASEDSTREAMS_BINARY_STREAM_H
