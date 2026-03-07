#ifndef BASEDLEX_LEXEME_H
#define BASEDLEX_LEXEME_H

#include <string>

#include "token.h"

namespace basedlex
{

  struct Lexeme
  {
    std::string text;
    Token token;
    int line;
    int column;
  };

} // namespace basedlex

#endif // BASEDLEX_LEXEME_H
