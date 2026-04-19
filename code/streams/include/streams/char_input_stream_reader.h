#ifndef BASEDSTREAMS_CHAR_INPUT_STREAM_READER_H
#define BASEDSTREAMS_CHAR_INPUT_STREAM_READER_H

#include <array>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <memory>
#include <optional>

#include "streams/char_input_stream.h"

namespace benson
{

  /// Buffers a Char_input_stream behind a 1024-codepoint heap-allocated scratch
  /// buffer and exposes sequential reads.
  class Char_input_stream_reader
  {
  public:
    explicit Char_input_stream_reader(Char_input_stream *stream)
        : _stream{stream},
          _scratch{std::make_unique<std::array<uint32_t, 1024>>()}
    {
    }

    std::optional<uint32_t> read()
    {
      if (_pos == _end)
      {
        _pos = 0;
        _end = _stream->read_characters(*_scratch);
        if (_end == 0)
        {
          return std::nullopt;
        }
      }
      return (*_scratch)[_pos++];
    }

  private:
    Char_input_stream *_stream;
    std::unique_ptr<std::array<uint32_t, 1024>> _scratch;
    std::ptrdiff_t _pos{};
    std::ptrdiff_t _end{};
  };

} // namespace benson

#endif // BASEDSTREAMS_CHAR_INPUT_STREAM_READER_H
