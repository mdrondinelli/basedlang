#ifndef BASEDLEX_BINARY_STREAM_H
#define BASEDLEX_BINARY_STREAM_H

#include <cstdint>
#include <optional>

namespace benson
{

  /// Abstract source of raw bytes. Implement this to feed binary data into a
  /// Utf8_char_stream.
  class Binary_stream
  {
  public:
    virtual ~Binary_stream() = default;

    /// Returns the next byte, or std::nullopt at EOF.
    virtual std::optional<uint8_t> read_byte() = 0;
  };

} // namespace benson

#endif // BASEDLEX_BINARY_STREAM_H
