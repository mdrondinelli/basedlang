#ifndef BASEDLEX_SOURCE_SPAN_H
#define BASEDLEX_SOURCE_SPAN_H

#include "source_location.h"

namespace benson
{

  /// A closed range of source positions [start, end].
  /// Both @c start and @c end are inclusive: @c start is the position of the
  /// first character and @c end is the position of the last character.
  struct Source_span
  {
    Source_location start; ///< Position of the first character (inclusive).
    Source_location end; ///< Position of the last character (inclusive).
  };

  /// Returns the smallest span whose @c start comes from @p begin and whose
  /// @c end comes from @p end. Use this to build the span of a composite
  /// construct from the spans of its outermost tokens or sub-nodes.
  Source_span hull(Source_span begin, Source_span end);

} // namespace benson

#endif // BASEDLEX_SOURCE_SPAN_H
