#ifndef BASEDLEX_LEXEME_H
#define BASEDLEX_LEXEME_H

#include <string>

#include "source_location.h"
#include "source_span.h"
#include "token.h"

namespace benson
{

  /// A single token produced by the lexer, including its text and source span.
  struct Lexeme
  {
    std::string text;
    Token token;
    Source_span span;
    /// Compatibility start location for existing call sites. Equal to
    /// @c span.start.
    Source_location location;
  };

  /// Returns the source span of a lexeme.
  /// @c start points to the first character of the lexeme text; @c end points
  /// to the last character.
  Source_span span_of(Lexeme const &lexeme);

} // namespace benson

#endif // BASEDLEX_LEXEME_H
