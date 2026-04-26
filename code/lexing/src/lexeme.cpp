#include "lexing/lexeme.h"

namespace benson
{

  Source_span span_of(Lexeme const &lexeme)
  {
    return lexeme.span;
  }

} // namespace benson
