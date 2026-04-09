#ifndef BASEDLEX_CHAR_STREAM_H
#define BASEDLEX_CHAR_STREAM_H

#include <cstdint>
#include <optional>

namespace bensonlex
{

  /// Abstract source of Unicode characters. Implement this to feed text into
  /// the lexer.
  class Char_stream
  {
  public:
    virtual ~Char_stream() = default;

    // Returns the Unicode codepoint of the next character, or std::nullopt at
    // EOF.
    virtual std::optional<uint32_t> read_character() = 0;
  };

} // namespace bensonlex

#endif // BASEDLEX_CHAR_STREAM_H
