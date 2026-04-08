#include "kalelex/lexeme_stream_reader.h"

#include "kalelex/lexeme_stream.h"

namespace kalelex
{

  Lexeme_stream_reader::Lexeme_stream_reader(Lexeme_stream *stream) noexcept
      : _stream{stream}
  {
  }

  Lexeme const &Lexeme_stream_reader::peek(int offset)
  {
    auto const uoffset = static_cast<std::size_t>(offset);
    while (_buffer.size() <= uoffset)
    {
      _buffer.push_back(_stream->lex());
    }
    return _buffer[uoffset];
  }

  Lexeme Lexeme_stream_reader::read()
  {
    if (_buffer.size() == 0)
    {
      return _stream->lex();
    }
    auto lexeme = std::move(_buffer[0]);
    _buffer.pop_front();
    return lexeme;
  }

} // namespace kalelex
