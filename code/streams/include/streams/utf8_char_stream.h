#ifndef BASEDSTREAMS_UTF8_CHAR_STREAM_H
#define BASEDSTREAMS_UTF8_CHAR_STREAM_H

#include <cstdint>
#include <optional>
#include <stdexcept>
#include <utility>

#include "streams/binary_stream.h"
#include "streams/char_stream.h"

namespace benson
{

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

    explicit Utf8_char_stream(Binary_stream *stream) noexcept
        : _stream{stream}
    {
    }

    std::optional<uint32_t> read_character() override
    {
      auto const b0 = _stream->read_byte();
      if (!b0)
      {
        return std::nullopt;
      }
      if ((*b0 & 0b1000'0000) == 0b0000'0000)
      {
        return *b0;
      }
      auto const [initial_codepoint, remaining] =
        [&]() -> std::pair<uint32_t, int>
      {
        if ((*b0 & 0b1110'0000) == 0b1100'0000)
        {
          return {*b0 & 0b0001'1111, 1};
        }
        if ((*b0 & 0b1111'0000) == 0b1110'0000)
        {
          return {*b0 & 0b0000'1111, 2};
        }
        if ((*b0 & 0b1111'1000) == 0b1111'0000)
        {
          return {*b0 & 0b0000'0111, 3};
        }
        throw Decode_error{};
      }();
      auto codepoint = initial_codepoint;
      for (auto i = 0; i < remaining; ++i)
      {
        auto const cont = _stream->read_byte();
        if (!cont || (*cont & 0b1100'0000) != 0b1000'0000)
        {
          throw Decode_error{};
        }
        codepoint = (codepoint << 6) | (*cont & 0b0011'1111);
      }
      return codepoint;
    }

  private:
    Binary_stream *_stream;
  };

} // namespace benson

#endif // BASEDSTREAMS_UTF8_CHAR_STREAM_H
