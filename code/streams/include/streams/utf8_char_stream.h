#ifndef BASEDSTREAMS_UTF8_CHAR_STREAM_H
#define BASEDSTREAMS_UTF8_CHAR_STREAM_H

#include <cstdint>
#include <optional>
#include <stdexcept>

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
      auto const read_continuation = [&](uint8_t min, uint8_t max) -> uint8_t
      {
        auto const cont = _stream->read_byte();
        if (!cont || *cont < min || *cont > max)
        {
          throw Decode_error{};
        }
        return *cont;
      };
      auto const read_tail = [&]() -> uint8_t
      {
        return read_continuation(0x80, 0xBF);
      };
      auto const decode_continuation = [](uint32_t codepoint, uint8_t byte)
      {
        return (codepoint << 6) | (byte & 0b0011'1111);
      };
      auto const codepoint = [&]() -> uint32_t
      {
        if (*b0 >= 0xC2 && *b0 <= 0xDF)
        {
          auto const b1 = read_tail();
          return decode_continuation(*b0 & 0b0001'1111, b1);
        }
        if (*b0 == 0xE0)
        {
          auto const b1 = read_continuation(0xA0, 0xBF);
          auto const b2 = read_tail();
          return decode_continuation(
            decode_continuation(*b0 & 0b0000'1111, b1),
            b2
          );
        }
        if ((*b0 >= 0xE1 && *b0 <= 0xEC) || (*b0 >= 0xEE && *b0 <= 0xEF))
        {
          auto const b1 = read_tail();
          auto const b2 = read_tail();
          return decode_continuation(
            decode_continuation(*b0 & 0b0000'1111, b1),
            b2
          );
        }
        if (*b0 == 0xED)
        {
          auto const b1 = read_continuation(0x80, 0x9F);
          auto const b2 = read_tail();
          return decode_continuation(
            decode_continuation(*b0 & 0b0000'1111, b1),
            b2
          );
        }
        if (*b0 == 0xF0)
        {
          auto const b1 = read_continuation(0x90, 0xBF);
          auto const b2 = read_tail();
          auto const b3 = read_tail();
          return decode_continuation(
            decode_continuation(decode_continuation(*b0 & 0b0000'0111, b1), b2),
            b3
          );
        }
        if (*b0 >= 0xF1 && *b0 <= 0xF3)
        {
          auto const b1 = read_tail();
          auto const b2 = read_tail();
          auto const b3 = read_tail();
          return decode_continuation(
            decode_continuation(decode_continuation(*b0 & 0b0000'0111, b1), b2),
            b3
          );
        }
        if (*b0 == 0xF4)
        {
          auto const b1 = read_continuation(0x80, 0x8F);
          auto const b2 = read_tail();
          auto const b3 = read_tail();
          return decode_continuation(
            decode_continuation(decode_continuation(*b0 & 0b0000'0111, b1), b2),
            b3
          );
        }
        throw Decode_error{};
      }();
      return codepoint;
    }

  private:
    Binary_stream *_stream;
  };

} // namespace benson

#endif // BASEDSTREAMS_UTF8_CHAR_STREAM_H
