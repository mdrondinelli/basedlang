#ifndef BASEDLEX_LEXEME_H
#define BASEDLEX_LEXEME_H

#include "source_span.h"
#include "spelling/spelling.h"
#include "token.h"

namespace benson
{

  /// A single token produced by the lexer, including its spelling and source
  /// span.
  struct Lexeme
  {
    Token token;
    Spelling spelling;
    Source_span span;
  };

  /// Returns the source span of a lexeme.
  /// @c start points to the first character of the lexeme text; @c end points
  /// to the last character.
  Source_span span_of(Lexeme const &lexeme);

} // namespace benson

#endif // BASEDLEX_LEXEME_H
