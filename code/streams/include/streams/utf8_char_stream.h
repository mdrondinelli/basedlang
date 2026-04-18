#ifndef BASEDSTREAMS_UTF8_CHAR_STREAM_H
#define BASEDSTREAMS_UTF8_CHAR_STREAM_H

#include <cstddef>
#include <cstdint>
#include <optional>
#include <span>
#include <stdexcept>

#include "streams/binary_stream_reader.h"
#include "streams/char_stream.h"

namespace benson
{

  /// Decodes UTF-8 bytes from a Binary_stream into Unicode codepoints.
  /// Buffers the underlying bytes through a Binary_stream_reader.
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

    explicit Utf8_char_stream(Binary_stream *stream)
        : _reader{stream}
    {
    }

    std::ptrdiff_t read_characters(std::span<uint32_t> buffer) override
    {
      auto const size = static_cast<std::ptrdiff_t>(buffer.size());
      for (auto i = std::ptrdiff_t{0}; i < size; ++i)
      {
        auto const codepoint = decode_codepoint();
        if (!codepoint)
        {
          return i;
        }
        buffer[i] = *codepoint;
      }
      return size;
    }

  private:
    std::optional<uint32_t> decode_codepoint()
    {
      auto const b0 = _reader.read();
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
        auto const cont = _reader.read();
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
    }

    Binary_stream_reader _reader;
  };

} // namespace benson

#endif // BASEDSTREAMS_UTF8_CHAR_STREAM_H
