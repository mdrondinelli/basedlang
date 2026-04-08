#ifndef BASEDLEX_LEXEME_STREAM_READER_H
#define BASEDLEX_LEXEME_STREAM_READER_H

#include "lexeme.h"
#include "ring_buffer.h"

namespace kalelex
{

  class Lexeme_stream;

  /// Wraps a Lexeme_stream with arbitrary lookahead, exposing peek/read
  /// operations backed by a growable ring buffer.
  class Lexeme_stream_reader
  {
  public:
    explicit Lexeme_stream_reader(Lexeme_stream *stream) noexcept;

    // The returned reference is invalidated by any subsequent call to peek or
    // read.
    Lexeme const &peek(int offset = 0);

    Lexeme read();

  private:
    Lexeme_stream *_stream;
    Ring_buffer<Lexeme> _buffer;
  };

} // namespace kalelex

#endif // BASEDLEX_LEXEME_STREAM_READER_H
