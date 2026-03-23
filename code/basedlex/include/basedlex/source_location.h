#ifndef BASEDLEX_SOURCE_LOCATION_H
#define BASEDLEX_SOURCE_LOCATION_H

#include <cstdint>

namespace basedlex
{

  /// A position in source code, given as a 1-based line and column number.
  struct Source_location
  {
    std::int32_t line; ///< 1-based line number.
    std::int32_t column; ///< 1-based column number.
  };

} // namespace basedlex

#endif // BASEDLEX_SOURCE_LOCATION_H
