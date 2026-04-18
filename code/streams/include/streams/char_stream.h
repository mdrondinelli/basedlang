#ifndef BASEDSTREAMS_CHAR_STREAM_H
#define BASEDSTREAMS_CHAR_STREAM_H

#include <cstddef>
#include <cstdint>
#include <optional>
#include <span>

namespace benson
{

  /// Abstract source of Unicode characters. Implement this to feed text into
  /// the lexer.
  class Char_stream
  {
  public:
    virtual ~Char_stream() = default;

    /// Reads up to buffer.size() codepoints into buffer and returns the number
    /// of codepoints read. Returns 0 at EOF. Implementations may return short
    /// reads.
    virtual std::ptrdiff_t read_characters(std::span<uint32_t> buffer) = 0;

    /// Returns the Unicode codepoint of the next character, or std::nullopt at
    /// EOF.
    std::optional<uint32_t> read_character()
    {
      auto buffer = uint32_t{};
      if (read_characters(std::span{&buffer, std::size_t{1}}) == 0)
      {
        return std::nullopt;
      }
      return buffer;
    }
  };

} // namespace benson

#endif // BASEDSTREAMS_CHAR_STREAM_H
