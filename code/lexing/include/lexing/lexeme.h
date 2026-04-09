#ifndef BASEDLEX_LEXEME_H
#define BASEDLEX_LEXEME_H

#include <string>

#include "source_location.h"
#include "token.h"

namespace benson
{

  /// A single token produced by the lexer, including its text and source
  /// location.
  struct Lexeme
  {
    std::string text;
    Token token;
    Source_location location;
  };

} // namespace benson

#endif // BASEDLEX_LEXEME_H
