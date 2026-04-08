#include <cassert>

#include "kalelex/char_stream_reader.h"

#include "kalelex/char_stream.h"

namespace kalelex
{

  Char_stream_reader::Char_stream_reader(Char_stream *stream) noexcept
      : _stream{stream}
  {
  }

  std::optional<uint32_t> Char_stream_reader::peek(std::ptrdiff_t offset)
  {
    assert(offset >= 0);
    auto const index = static_cast<std::size_t>(offset);
    buffer_to(index + 1);
    if (index < _buffer.size())
    {
      return _buffer[index];
    }
    return std::nullopt;
  }

  std::optional<uint32_t> Char_stream_reader::read()
  {
    buffer_to(1);
    if (_buffer.empty())
    {
      return std::nullopt;
    }
    auto const result = _buffer.front();
    _buffer.erase(_buffer.begin());
    return result;
  }

  void Char_stream_reader::buffer_to(std::size_t count)
  {
    while (_buffer.size() < count)
    {
      auto const c = _stream->read_character();
      if (!c)
      {
        return;
      }
      _buffer.push_back(*c);
    }
  }

} // namespace kalelex
