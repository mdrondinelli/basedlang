#ifndef BASEDSTREAMS_ISTREAM_INPUT_STREAM_H
#define BASEDSTREAMS_ISTREAM_INPUT_STREAM_H

#include <cstddef>
#include <istream>
#include <span>

#include "streams/input_stream.h"

namespace benson
{

  class Istream_input_stream: public Input_stream
  {
  public:
    explicit Istream_input_stream(std::istream *stream) noexcept
        : _stream{stream}
    {
    }

    std::ptrdiff_t read_bytes(std::span<std::byte> buffer) override
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

#endif // BASEDSTREAMS_ISTREAM_INPUT_STREAM_H
