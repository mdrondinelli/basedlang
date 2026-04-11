#include "lexing/lexeme.h"

namespace benson
{

  Source_span span_of(Lexeme const &lexeme)
  {
    return lexeme.span;
  }

  Source_span hull(Source_span begin, Source_span end)
  {
    return Source_span{.start = begin.start, .end = end.end};
  }

} // namespace benson
