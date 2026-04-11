#include "lexing/lexeme.h"

namespace benson
{

  Source_span span_of(Lexeme const &lexeme)
  {
    auto const end = Source_location{
      .line = lexeme.location.line,
      .column = lexeme.location.column +
                static_cast<std::int32_t>(lexeme.text.size()) - 1,
    };
    return Source_span{.start = lexeme.location, .end = end};
  }

  Source_span hull(Source_span begin, Source_span end)
  {
    return Source_span{.start = begin.start, .end = end.end};
  }

} // namespace benson
