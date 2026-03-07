#ifndef BASEDLEX_LEXER_H
#define BASEDLEX_LEXER_H

#include <variant>

#include "char_stream_reader.h"
#include "lexeme.h"

namespace basedlex
{

  class Lexer
  {
  public:
    struct Lex_error
    {
      int line;
      int column;
    };

    using Lex_result = std::variant<Lexeme, Lex_error>;

    explicit Lexer(Char_stream *stream) noexcept;

    Lex_result lex();

  private:
    Char_stream_reader _reader;
    int _line;
    int _column;
  };

} // namespace basedlex

#endif // BASEDLEX_LEXER_H
