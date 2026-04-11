#include "lexing/utf8_char_stream.h"

#include "lexing/binary_stream.h"

namespace benson
{

  Utf8_char_stream::Utf8_char_stream(Binary_stream *stream) noexcept
      : _stream{stream}
  {
  }

  std::optional<uint32_t> Utf8_char_stream::read_character()
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

} // namespace benson
