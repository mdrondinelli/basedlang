#ifndef BASEDLEX_LEXEME_H
#define BASEDLEX_LEXEME_H

#include <string>

#include "token.h"

namespace basedlex
{

  /// A single token produced by the lexer, including its text and source
  /// location.
  struct Lexeme
  {
    std::string text;
    Token token;
    int line;
    int column;
  };

} // namespace basedlex

#endif // BASEDLEX_LEXEME_H
