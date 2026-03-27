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
      explicit Lex_error(Source_location location)
          : std::runtime_error{"lex error"}, location{location}
      {
      }

      Source_location location;
    };

    explicit Lexeme_stream(Char_stream *stream) noexcept;

    Lexeme lex();

  private:
    Char_stream_reader _reader;
    Source_location _location;
  };

} // namespace basedlex

#endif // BASEDLEX_LEXEME_STREAM_H
