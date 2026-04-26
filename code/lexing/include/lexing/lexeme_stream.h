#ifndef BASEDLEX_LEXEME_STREAM_H
#define BASEDLEX_LEXEME_STREAM_H

#include <stdexcept>

#include <spelling/spelling.h>
#include <streams/char_input_stream.h>
#include <streams/char_input_stream_peeker.h>
#include <source/source_location.h>

#include "lexeme.h"

namespace benson
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

    Lexeme_stream(Char_input_stream *stream, Spelling_table *spellings);

    Lexeme lex();

  private:
    void consume_newline();

    char32_t consume_non_newline();

    Char_input_stream_peeker _reader;
    Spelling_table *_spellings;
    Source_location _location;
  };

} // namespace benson

#endif // BASEDLEX_LEXEME_STREAM_H
