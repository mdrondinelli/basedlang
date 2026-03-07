#include "basedlex/char_stream_reader.h"

#include "basedlex/char_stream.h"

namespace basedlex
{

  Char_stream_reader::Char_stream_reader(Char_stream *stream) noexcept
      : _stream{stream}, _next{stream->read_character()}
  {
  }

  std::optional<uint32_t> Char_stream_reader::peek() const noexcept
  {
    return _next;
  }

  std::optional<uint32_t> Char_stream_reader::read()
  {
    auto const result = _next;
    _next = _stream->read_character();
    return result;
  }

} // namespace basedlex
