#ifndef BASEDLEX_UTF8_CHAR_STREAM_H
#define BASEDLEX_UTF8_CHAR_STREAM_H

#include <cstdint>
#include <optional>
#include <stdexcept>

#include "char_stream.h"

namespace bensonlex
{

  class Binary_stream;

  /// Decodes UTF-8 bytes from a Binary_stream into Unicode codepoints.
  /// Throws Decode_error on invalid byte sequences.
  class Utf8_char_stream: public Char_stream
  {
  public:
    class Decode_error: public std::runtime_error
    {
    public:
      Decode_error()
          : std::runtime_error{"invalid UTF-8 byte sequence"}
      {
      }
    };

    explicit Utf8_char_stream(Binary_stream *stream) noexcept;

    std::optional<uint32_t> read_character() override;

  private:
    Binary_stream *_stream;
  };

} // namespace bensonlex

#endif // BASEDLEX_UTF8_CHAR_STREAM_H
