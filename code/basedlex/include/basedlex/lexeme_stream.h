#ifndef BASEDLEX_LEXEME_STREAM_H
#define BASEDLEX_LEXEME_STREAM_H

#include <stdexcept>

#include "char_stream_reader.h"
#include "lexeme.h"

namespace basedlex
{

  class Lexeme_stream
  {
  public:
    class Lex_error: public std::runtime_error
    {
    public:
      Lex_error(int line, int column)
          : std::runtime_error{"lex error"}, line{line}, column{column}
      {
      }

      int line;
      int column;
    };

    explicit Lexeme_stream(Char_stream *stream) noexcept;

    Lexeme lex();

  private:
    Char_stream_reader _reader;
    int _line;
    int _column;
  };

} // namespace basedlex

#endif // BASEDLEX_LEXEME_STREAM_H
