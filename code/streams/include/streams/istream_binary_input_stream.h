#ifndef BASEDSTREAMS_ISTREAM_BINARY_INPUT_STREAM_H
#define BASEDSTREAMS_ISTREAM_BINARY_INPUT_STREAM_H

#include <cstddef>
#include <cstdint>
#include <istream>
#include <span>

#include "streams/binary_input_stream.h"

namespace benson
{

  class Istream_binary_input_stream: public Binary_input_stream
  {
  public:
    explicit Istream_binary_input_stream(std::istream *stream) noexcept
        : _stream{stream}
    {
    }

    std::ptrdiff_t read_bytes(std::span<uint8_t> buffer) override
    {
      if (buffer.empty())
      {
        return 0;
      }
      _stream->read(
        reinterpret_cast<char *>(buffer.data()),
        static_cast<std::streamsize>(buffer.size())
      );
      return static_cast<std::ptrdiff_t>(_stream->gcount());
    }

  private:
    std::istream *_stream;
  };

} // namespace benson

#endif // BASEDSTREAMS_ISTREAM_BINARY_INPUT_STREAM_H
