#ifndef BASEDSTREAMS_ISTREAM_BINARY_STREAM_H
#define BASEDSTREAMS_ISTREAM_BINARY_STREAM_H

#include <cstdint>
#include <istream>
#include <optional>

#include "streams/binary_stream.h"

namespace benson
{

  class Istream_binary_stream: public Binary_stream
  {
  public:
    explicit Istream_binary_stream(std::istream *stream) noexcept
        : _stream{stream}
    {
    }

    std::optional<uint8_t> read_byte() override
    {
      auto const c = _stream->get();
      if (c == std::istream::traits_type::eof())
      {
        return std::nullopt;
      }
      return static_cast<uint8_t>(c);
    }

  private:
    std::istream *_stream;
  };

} // namespace benson

#endif // BASEDSTREAMS_ISTREAM_BINARY_STREAM_H
