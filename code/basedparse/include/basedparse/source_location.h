#ifndef BASEDPARSE_SOURCE_LOCATION_H
#define BASEDPARSE_SOURCE_LOCATION_H

#include <cstdint>

namespace basedparse
{

  struct Source_location
  {
    std::int32_t line;
    std::int32_t column;
  };

} // namespace basedparse

#endif // BASEDPARSE_SOURCE_LOCATION_H
