#ifndef BASEDLEX_LEXER_INPUT_STREAM_H
#define BASEDLEX_LEXER_INPUT_STREAM_H

#include <cstdint>
#include <optional>

namespace basedlex
{

  class Lexer_input_stream
  {
  public:
    virtual ~Lexer_input_stream() = default;

    // Returns the Unicode codepoint of the next character, or std::nullopt at
    // EOF.
    virtual std::optional<uint32_t> read_character() = 0;
  };

} // namespace basedlex

#endif // BASEDLEX_LEXER_INPUT_STREAM_H
